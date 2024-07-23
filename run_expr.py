from itertools import product
import pandas as pd

K = 3
Q = 8


def run_query(expr, sid, tid, exec):
    import os
    import subprocess

    if not os.path.exists("./output"):
        os.makedirs("output")

    cmd = f"{exec} {expr} {sid} {tid} {K} {Q}"
    print(f"Run [{cmd}]")
    subprocess.run(cmd.split())


def run_map(expr: str):
    print(f">>> Running {expr}")
    mapname = expr.removesuffix(".csv").split("/")[-1]
    logdir = "./output"
    header = "map,s,t,K,Q,algo,best,runtime"
    with open(f"{logdir}/{mapname}.log", "w") as f:
        f.write(header + "\n")
    df = pd.read_csv(expr).rename(columns=lambda x: x.strip())
    ids = set(df["index_from"]) | set(df["index_to"])  # type: ignore
    for i, j in product(list(ids), list(ids)):
        if i >= j:
            continue
        run_query(expr, i, j, "./build/run_refill")
        run_query(expr, j, i, "./build/run_refill")
        run_query(expr, i, j, "./build/dp")
        run_query(expr, j, i, "./build/dp")
        run_query(expr, i, j, "./build/mip-gurobi")
        run_query(expr, j, i, "./build/mip-gurobi")


def run_small():
    exprs = [
        "./small-data/graph_data.csv",
        "./small-data/graph_data2.csv",
        "./small-data/graph_data3.csv",
        "./small-data/graph_data4.csv",
        "./small-data/graph_data5.csv",
    ]

    for expr in exprs:
        run_map(expr)


if __name__ == "__main__":
    run_small()
