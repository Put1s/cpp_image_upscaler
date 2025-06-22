#!/usr/bin/env python3
import os
import sys
import glob
import subprocess
import argparse
import numpy as np

def parse_args():
    p = argparse.ArgumentParser(
        description="Run Upscaler in metrics+quiet mode over a folder of images and compute summary statistics"
    )
    p.add_argument("images_dir", help="Папка с изображениями")
    p.add_argument(
        "--exe", "-e", default="./Upscaler",
        help="Путь к исполняемому файлу Upscaler"
    )
    p.add_argument(
        "--algo", "-a", default="bicubic",
        choices=["bilinear", "bicubic", "edsr", "espcn", "fsrcnn", "lapsrn"],
        help="Алгоритм апскейла"
    )
    p.add_argument(
        "--scale", "-s", type=int, default=2, choices=[2,3,4,8],
        help="Коэффициент масштаба"
    )
    p.add_argument(
        "--model", "-m", default="",
        help="Путь к файлу модели (.pb) для DNN-алгоритмов"
    )
    return p.parse_args()

def run_one(exe, img_path, algo, scale, model):
    cmd = [exe, img_path, algo, str(scale)]
    if model:
        cmd.append(model)
    cmd += ["--metrics", "--quiet"]
    proc = subprocess.run(cmd, capture_output=True, text=True)
    if proc.returncode != 0:
        print(f"Ошибка для {img_path}:\n{proc.stderr}", file=sys.stderr)
        return None
    parts = proc.stdout.strip().split()
    return list(map(float, parts))

def main():
    args = parse_args()

    pattern = os.path.join(args.images_dir, "*")
    files = sorted(f for f in glob.glob(pattern) if os.path.isfile(f))
    if not files:
        print("Нет файлов в папке", args.images_dir, file=sys.stderr)
        sys.exit(1)

    metrics = []  # [psnr, ssim_b, ssim_g, ssim_r]
    for img in files:
        res = run_one(args.exe, img, args.algo, args.scale, args.model)
        if res is not None:
            metrics.append(res)

    if not metrics:
        print("Не удалось получить ни одной метрики.", file=sys.stderr)
        sys.exit(1)

    arr = np.array(metrics)  # shape = (N, 4)
    names = ["PSNR (dB)", "SSIM B", "SSIM G", "SSIM R"]

    print("Статистика по", len(arr), "изображениям:")
    for i, name in enumerate(names):
        col = arr[:, i]
        print(f"─ {name:10s} : "
              f"mean={col.mean():.4f}, "
              f"min={col.min():.4f}, "
              f"max={col.max():.4f}, "
              f"median={np.median(col):.4f}, "
              f"std={col.std(ddof=1):.4f}")

if __name__ == "__main__":
    main()
