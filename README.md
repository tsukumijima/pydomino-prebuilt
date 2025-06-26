# pydomino-prebuilt

[![PyPI](https://img.shields.io/pypi/v/pydomino-prebuilt.svg)](https://pypi.python.org/pypi/pydomino-prebuilt)
[![Build and Publish Wheels](https://github.com/tsukumijima/pydomino-prebuilt/actions/workflows/build-wheels.yml/badge.svg)](https://github.com/tsukumijima/pydomino-prebuilt/actions/workflows/build-wheels.yml)
[![License](https://img.shields.io/badge/license-MIT-brightgreen.svg?style=flat)](LICENSE.md)

pydomino-prebuilt は、[pydomino](https://github.com/DwangoMediaVillage/pydomino) の事前ビルド済み wheels を PyPI に公開することを目的とした、[pydomino](https://github.com/DwangoMediaVillage/pydomino) の派生ライブラリです。

## Changes in this fork

- **パッケージ名を `pydomino-prebuilt` に変更**
  - ライブラリ名は `pydomino` から変更されておらず、[pydomino](https://github.com/DwangoMediaVillage/pydomino) 本家同様に `import pydomino` でインポートできる
  - [pydomino](https://github.com/DwangoMediaVillage/pydomino) 本家のドロップイン代替として利用できる
- **Windows・macOS (x64 / arm64)・Linux すべての事前ビルド済み wheels を PyPI に公開**
  - pydomino は C++ や CMake 、サブモジュールに依存しており、ビルド環境の構築難易度が比較的高い上に、PyPI には公開されていない
    - 特に Windows においては MSVC のインストールが必要となる
  - 事前ビルド済みの wheels を PyPI に公開することで、ビルド環境のない PC でも簡単にインストール可能にすることを意図している
- **明示的に Python 3.11 / 3.12 / 3.13 をサポート対象に追加**
- **ONNX モデルを Wheel に同梱**
  - 学習済みモデル `phoneme_transition_model.onnx` は Wheel パッケージに同梱されており、pydomino-prebuilt と同時にインストールされる
  - これにより、利用者が毎回モデルファイルをダウンロードしたり、ファイルパスを指定する必要がなくなった
- **Python API の使いやすさを向上**
  - `pydomino.Aligner()` のコンストラクタで `onnxfile` パラメータを省略可能に変更した
  - `onnxfile` パラメータが指定されていない場合、自動的に内蔵の学習済みモデルが利用される
  - 従来通り、カスタムモデルのパス指定も可能
- **CLI の使いやすさを向上**
  - `domino` コマンドで `--onnx_path` オプションを省略可能に変更した
  - `--onnx_path` オプションが指定されていない場合、自動的に内蔵の学習済みモデルが利用される
  - 従来通り、カスタムモデルのパス指定も可能
- **パッケージメタデータの整理**
  - `pyproject.toml` にプロジェクトメタデータを統合し、現代的な Python パッケージ標準に準拠した
  - 適切な classifiers を追加して、対応 OS・Python バージョン・ライセンス情報などを明示した

## Installation

下記コマンドを実行して、ライブラリをインストールできます。

```bash
pip install pydomino-prebuilt
```

下記ならびに [docs/](docs/) 以下のドキュメントは、[pydomino](https://github.com/DwangoMediaVillage/pydomino) 本家のドキュメントを改変なしでそのまま引き継いでいます。  
これらのドキュメントの内容が pydomino-prebuilt にも通用するかは保証されません。

-------

# pydomino

`pydomino` は日本語音声に対して音素ラベルをアラインメントするためのツールです。GPUは不要です。
ライブラリとして Python から使うこともコマンドラインツールとしてコンソールから使うこともできます。
ドキュメントは [こちら](https://dwangomediavillage.github.io/pydomino/) からご覧いただけます。
技術の解説記事は [こちら](https://dmv.nico/ja/articles/domino_phoneme_transition/) からご覧いただけます

## Installation

### Requisites

- CMake
- Python >= 3.10 (miniconda etc.)
- Visual Studio >= 2019 (for Windows)

### Build & Install

#### Linux / Mac

```sh
git clone --recursive https://github.com/DwangoMediaVillage/pydomino
cd pydomino
pip install ./
```

また、下記のように直接 pip インストールもできます（コマンドラインツールもインストールされます。コマンドラインツールを使うときは、onnxファイルを指定する必要があります。`onnx_model/phoneme_transition_model.onnx` をダウンロードしてください）：

```sh
pip install git+https://github.com/DwangoMediaVillage/pydomino
```


#### Windows

`Anaconda Prompt (miniconda3)` 環境において MSVC の `vcvars64.bat` を利用してインストールします：

* `"C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat"` or
* `"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"`.

```sh
# on `Anaconda Prompt (miniconda3)`
"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
git clone --recursive https://github.com/DwangoMediaVillage/pydomino
cd pydomino
pip install ./
```

また、下記のように直接 pip インストールもできます（コマンドラインツールもインストールされます。コマンドラインツールを使うときは、onnxファイルを指定する必要があります。`onnx_model/phoneme_transition_model.onnx` をダウンロードしてください）：

```sh
pip install git+https://github.com/DwangoMediaVillage/pydomino
```

## Run samples

### Python Library

```py
alignmer: pydomino.Aligner = pydomino.Aligner(path-to-model-file.onnx)

y: np.ndarray = librosa.load(path-to-wav-file, sr=16_000, mono=True, dtype=np.float32)[0]
p: list[str] = path-to-phoneme-file.read_text().split(" ")
z: list[tuple[float, float, str]] = alignmer.align(y, " ".join(p), 3) # [(start_time_sec, end_time_sec, phoneme_str)]
```

* `path-to-model-file.onnx` は事前学習済みの onnx モデルファイルです。
  * `onnx_model/phoneme_transition_model.onnx`にあります。
* `path-to-wav-file` はサンプリング周波数 16kHz のモノラル wav ファイルです。
* `path-to-phoneme-file` は音素を空白区切りしたテキストが格納されたファイルのパスです。
  * NOTE: 開始音素と終了音素は `pau` である必要があります。

`phonemes` に使える音素一覧は下記の通りです：

|       |      |     |      |     |      |     |      |      |      |
| ----- | ---- | --- | ---- | --- | ---- | --- | ---- | ---- | ---- |
| `pau` | `ry` | `r` | `my` | `m` | `ny` | `n` | `j`  | `z`  | `by` |
| `b`   | `dy` | `d` | `gy` | `g` | `ky` | `k` | `ch` | `ts` | `sh` |
| `s`   | `hy` | `h` | `v`  | `f` | `py` | `p` | `t`  | `y`  | `w`  |
| `N`   | `a`  | `i` | `u`  | `e` | `o`  | `I` | `U`  | `cl` |      |

### Console Application

上記のインストール手順における `pip install` により Cli ツールも自動でビルドされます。

ビルドされたツールは下記のようにして使えます：

```sh
domino \
    --input_path={path-to-wav-file} \
    --input_phoneme={path-to-phoneme-file} \
    --output_path={path-to-output-lab-file} \
    --onnx_path={path-to-output-onnx-file} \
    --min_frame==3
```

onnxファイルは当組織で学習済みの `onnx_model/phoneme_transition_model.onnx` を用意していますのでお使いください

### label file format (.lab)

アラインメント結果のラベルファイル (.lab) は、tsv ファイル構造になっています。

各行に音素の開始時刻と終了時刻 (いずれも単位は秒) と、そのときの音素が TAB 区切りで並んでいます：

```txt
0.000	0.110	pau
0.110	0.140	d
0.140	0.170	o
0.170	0.210	w
0.210	0.360	a
0.360	0.450	N
0.450	0.490	g
0.490	0.620	o
0.620	0.755	pau
```
