import sys
from itertools import product
import pandas as pd

K = 3
Q = 8


def run_query(expr, sid, tid, exec):
    import subprocess

    cmd = f"{exec} {expr} {sid} {tid} {K} {Q}"
    print(f"Run [{cmd}]")
    subprocess.run(cmd.split())


def run_map(expr: str, solvers):
    import os
    print(f">>> Running {expr}")
    mapname = expr.removesuffix(".csv").split("/")[-1]
    logdir = "./output"
    header = "map,s,t,K,Q,algo,best,runtime"

    if not os.path.exists("./output"):
        os.makedirs("output")
    with open(f"{logdir}/{mapname}.log", "w") as f:
        f.write(header + "\n")
    df = pd.read_csv(expr).rename(columns=lambda x: x.strip())
    ids = set(df["index_from"]) | set(df["index_to"])  # type: ignore
    for i, j in product(list(ids), list(ids)):
        if i >= j:
            continue
        for solver in solvers:
            run_query(expr, i, j, f"./build/{solver}")
            run_query(expr, j, i, f"./build/{solver}")


def run_city():
    global K, Q
    K = 10
    Q = 60000
    exprs = [
        "./city-data/Austin_gas.csv",
        "./city-data/London_gas.csv",
        "./city-data/Moscow_no_zero_dist.csv",
        "./city-data/Phil_gas.csv",
        "./city-data/Phoenix_gas.csv",
    ]
    solvers = ["run_refill", "dp"]
    for expr in exprs:
        run_map(expr, solvers)


def run_small():
    global K, Q
    K = 3
    Q = 8 
    exprs = [
        "./small-data/graph_data.csv",
        "./small-data/graph_data2.csv",
        "./small-data/graph_data3.csv",
        "./small-data/graph_data4.csv",
        "./small-data/graph_data5.csv",
    ]

    solvers = ["dp", "run_refill", "mip-gurobi"]
    for expr in exprs:
        run_map(expr, solvers)


if __name__ == "__main__":
    if len(sys.argv)>1 and sys.argv[1] == "city":
        run_city()
    else:
        run_small()
