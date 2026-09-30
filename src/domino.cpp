/* Pythonライブラリとコマンドコンソール両方のインタフェースを書く場所 */
#include "domino.hpp"

#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <sstream>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "phoneme_transition.hpp"
#include "viterbi.hpp"

namespace domino {
Aligner::Aligner(std::string const &path, int const N)
    : env_(),
      session_options_(),
      session_(env_, std::filesystem::path(path).c_str(), session_options_),
      memory_info_(Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU)),
      run_options_(),
      N_(N) {
  std::cout << "path: " << path << std::endl;
}

Aligner::~Aligner() { this->release(); }

void Aligner::release() {
  run_options_.release();
  memory_info_.release();
  session_.release();
  env_.release();
}

std::vector<std::tuple<double, double, std::string>> Aligner::align_phonemes(Eigen::Ref<Eigen::VectorXf> const wav_data,
                                                                             std::string const &phonemes, int N) {
  return this->align(wav_data.data(), wav_data.size(), Aligner::read_phonemes(phonemes), N);
}

std::vector<std::tuple<double, double, std::string>> Aligner::align(float const *wav_data,
                                                                    std::size_t const wav_data_size,
                                                                    std::vector<int> const &token_ids,
                                                                    int min_timeframe_per_1_phoneme) {
  constexpr char const *const input_names[] = {"input_waveform"};
  constexpr char const *const output_names[] = {"transition_logprobs", "blank_logprobs"};
  // NOTE: C++17以上が必須

  // 直前と同じ音声なら音響モデルを実行せず、保持している出力を使う (キャッシュは複数スレッドからの呼び出しに備えてロックで守る)
  std::lock_guard<std::mutex> const lock(cache_mutex_);
  bool const is_cached_wav = cached_wav_.size() == wav_data_size &&
                             std::equal(cached_wav_.begin(), cached_wav_.end(), wav_data);
  if (!is_cached_wav) {
    std::array<std::int64_t, 2> const wav_data_shape = {1, static_cast<std::int64_t>(wav_data_size)};
    Ort::Value inputs[] = {
        Ort::Value::CreateTensor(memory_info_, const_cast<float *>(wav_data), wav_data_size, wav_data_shape.data(),
                                 wav_data_shape.size()),
    };
    std::vector<Ort::Value> const outputs =
        session_.Run(run_options_, input_names, inputs, std::size(input_names), output_names, std::size(output_names));
    auto const transition_logprobs_shape = outputs[0].GetTensorTypeAndShapeInfo().GetShape();
    // 出力を先に写してから音声を記録し、途中で例外が出ても不完全なキャッシュを一致と判定しないようにする
    cached_wav_.clear();
    cached_num_timeframe_ = transition_logprobs_shape[1];
    cached_num_transition_ = transition_logprobs_shape[2];
    float const *const transition_logprobs = outputs[0].GetTensorData<float>();
    float const *const blank_logprobs = outputs[1].GetTensorData<float>();
    cached_transition_logprobs_.assign(transition_logprobs,
                                       transition_logprobs + cached_num_timeframe_ * cached_num_transition_);
    cached_blank_logprobs_.assign(blank_logprobs, blank_logprobs + cached_num_timeframe_);
    cached_wav_.assign(wav_data, wav_data + wav_data_size);
  }
  float const *const transition_logprobs = cached_transition_logprobs_.data();
  float const *const blank_logprobs = cached_blank_logprobs_.data();

  int const num_timeframe = cached_num_timeframe_;
  if (min_timeframe_per_1_phoneme * (token_ids.size() - 1) + 1 > num_timeframe) {
    std::cout << "[warn] timeframe / phoneme is too large for alignment. " << std::endl;
    min_timeframe_per_1_phoneme = (num_timeframe - 1) / (token_ids.size() - 1);
  }
  std::vector<int> transition_timeframes(token_ids.size(), 0);
  solve_viterbi(cached_num_timeframe_, cached_num_transition_, transition_logprobs, blank_logprobs,
                min_timeframe_per_1_phoneme, token_ids, transition_timeframes);

  // 音素遷移トークンの予測発生時刻を音素ラベル表現の形に変換する
  std::vector<std::tuple<double, double, std::string>> alignment;
  float begin_sec = 0.0;
  std::vector<std::string> phonemes = tokenizer.to_phonemes(token_ids);
  for (int i = 0; i < token_ids.size(); ++i) {
    alignment.push_back(std::make_tuple(begin_sec, float(transition_timeframes[i]) / 100, phonemes[i]));
    begin_sec = float(transition_timeframes[i]) / 100;
  }
  alignment.push_back(std::make_tuple(begin_sec, float(wav_data_size) / 16000, phonemes[token_ids.size()]));
  return alignment;
}

std::vector<int> Aligner::read_phonemes(std::filesystem::path const &file) {
  if (!std::filesystem::is_regular_file(file)) {
    throw std::runtime_error("file is not regular file: " + file.string());
  }
  std::ifstream ss{file};
  return tokenizer.read_phonemes(ss);
}

std::vector<int> Aligner::read_phonemes(std::string const &s) {
  std::istringstream ss{s};
  return tokenizer.read_phonemes(ss);
}
}  // namespace domino
