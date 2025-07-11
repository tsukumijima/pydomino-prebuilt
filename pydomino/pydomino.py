import numpy as np
from pathlib import Path
from pydomino.pydomino_cpp import Aligner_cpp


class Aligner(Aligner_cpp):
    def __init__(self, onnxfile: str | None = None) -> None:
        """
        コンストラクタ。ここで `onnxfile` で指定した ONNX ファイルを読み込む。

        Args:
            onnxfile (str | None): 読み込ませたい ONNX ファイルパス (None の場合は内蔵モデルを使用)
        """
        if onnxfile is None:
            # デフォルトの内蔵モデルを使用
            default_model_path = (
                Path(__file__).parent / "onnx_model" / "phoneme_transition_model.onnx"
            )
            if not default_model_path.exists():
                raise FileNotFoundError(
                    f"Default model not found at {default_model_path}. "
                    "Please specify onnxfile parameter explicitly."
                )
            onnxfile = str(default_model_path)

        super().__init__(onnxfile)

    def __del__(self) -> None:
        super().release()

    def align(
        self, waveform_mono_16kHz: np.ndarray, phonemes: str, min_aligned_timeframe: int
    ) -> list[tuple[float, float, str]]:
        """
        音素遷移予測に基づく日本語音素アラインメントを実行する関数。

        Args:
            waveform_mono_16kHz (np.ndarray): 16kHzのモノラル音声信号。サンプリング値は (-1, 1) に正規化された32bit浮動小数点
            phonemes (str): 半角スペース区切りの音素列
            min_aligned_timeframe (int): 両端にある `pau` 音素以外のすべての音素に割り当てられる最低時間フレーム。1フレーム10ミリ秒なので、N=3ですべての音素が30ミリ秒以上割り当てられる

        Returns:
            list[tuple[float, float, str]]: アラインメント結果。`(開始秒数, 終了秒数, 音素)` のタプル列
        """
        return super().align(waveform_mono_16kHz, phonemes, min_aligned_timeframe)

    def release(self) -> None:
        """内部で読み込んだ ONNX ファイルのメモリを開放する関数。デストラクタでこの関数を呼び出す。"""
        super().release()
