import os
import sys
from pathlib import Path


def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    if os.name == "nt":
        binary_path = os.path.join(script_dir, "domino.exe")
    else:
        binary_path = os.path.join(script_dir, "domino")

    if not os.path.exists(binary_path):
        print(f"Error: {binary_path} not found")
        sys.exit(1)

    # onnx_path が指定されていない場合はデフォルトモデルパスを追加
    args = sys.argv[1:]
    if "--onnx_path" not in args:
        default_model_path = (
            Path(script_dir) / "onnx_model" / "phoneme_transition_model.onnx"
        )
        if default_model_path.exists():
            args.extend(["--onnx_path", str(default_model_path)])
        else:
            print(f"Error: Default model not found at {default_model_path}")
            print("Please specify --onnx_path explicitly")
            sys.exit(1)

    os.execvp(binary_path, [binary_path] + args)
