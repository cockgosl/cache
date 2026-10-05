import os

import matplotlib.pyplot as plt
import pandas as pd


CSV_PATH = "benchmarks/benchmarks.csv"
OUT_DIR = "benchmarks/plots"


WORKLOADS = [
    "HOT_COLD",
    "LOOP",
    "MIXED",
    "RANDOM",
    "WORKING_SET"
]


def plot_single_cache(df):
    data = df[
        df["Mode"] == "Single"
    ].copy()

    algorithms = data["Algorithm"].unique()
    workloads = data["Workload"].unique()

    x = range(len(workloads))
    width = 0.15

    plt.figure(figsize=(14, 7))

    for i, algorithm in enumerate(algorithms):
        algorithm_data = data[
            data["Algorithm"] == algorithm
        ]

        values = []

        for workload in workloads:
            row = algorithm_data[
                algorithm_data["Workload"] == workload
            ]

            if row.empty:
                values.append(0)
            else:
                values.append(
                    row.iloc[0]["Hit_Ratio_Pct"]
                )

        positions = [
            value + (i - len(algorithms) / 2) * width
            for value in x
        ]

        plt.bar(
            positions,
            values,
            width=width,
            label=algorithm
        )

    plt.xticks(
        list(x),
        workloads,
        rotation=45
    )

    plt.xlabel("Workload")
    plt.ylabel("Hit ratio (%)")
    plt.title("Single Cache Hit Ratio")

    plt.legend(
        title="Algorithm"
    )

    plt.grid(
        axis="y",
        alpha=0.3
    )

    plt.tight_layout()

    plt.savefig(
        os.path.join(
            OUT_DIR,
            "single_cache_comparison.png"
        ),
        dpi=200
    )

    plt.close()


def prepare_multi_data(df, levels, mode):
    data = df[
        (df["Mode"] == mode) &
        (df["Workload"].isin(WORKLOADS))
    ].copy()

    if levels == 2:
        data = data[
            data["L3_Capacity"] == 0
        ].copy()
    else:
        data = data[
            data["L3_Capacity"] > 0
        ].copy()

    grouped = data.groupby(
        "Algorithm",
        as_index=False
    ).agg({
        "L1_Hits": "sum",
        "L2_Hits": "sum",
        "L3_Hits": "sum",
        "Ideal_Hits": "sum"
    })

    grouped["L1_Pct"] = (
        100.0 *
        grouped["L1_Hits"] /
        grouped["Ideal_Hits"]
    )

    grouped["L2_Pct"] = (
        100.0 *
        grouped["L2_Hits"] /
        grouped["Ideal_Hits"]
    )

    grouped["L3_Pct"] = 0.0

    if levels == 3:
        grouped["L3_Pct"] = (
            100.0 *
            grouped["L3_Hits"] /
            grouped["Ideal_Hits"]
        )

    grouped["Total_Pct"] = (
        grouped["L1_Pct"] +
        grouped["L2_Pct"] +
        grouped["L3_Pct"]
    )

    grouped = grouped.sort_values(
        "Total_Pct",
        ascending=False
    )

    return grouped


def plot_multi(df, levels, mode, top_n=None):
    grouped = prepare_multi_data(
        df,
        levels,
        mode
    )

    if top_n is not None:
        grouped = grouped.head(top_n)

    grouped = grouped.iloc[::-1]

    if top_n is None:
        if levels == 2:
            height = 14
        else:
            height = 32
    else:
        height = 8

    plt.figure(
        figsize=(14, height)
    )

    algorithms = grouped["Algorithm"]

    plt.barh(
        algorithms,
        grouped["L1_Pct"],
        label="L1"
    )

    plt.barh(
        algorithms,
        grouped["L2_Pct"],
        left=grouped["L1_Pct"],
        label="L2"
    )

    if levels == 3:
        plt.barh(
            algorithms,
            grouped["L3_Pct"],
            left=(
                grouped["L1_Pct"] +
                grouped["L2_Pct"]
            ),
            label="L3"
        )

    for _, row in grouped.iterrows():
        l1_position = row["L1_Pct"] / 2

        l2_position = (
            row["L1_Pct"] +
            row["L2_Pct"] / 2
        )

        l3_position = (
            row["L1_Pct"] +
            row["L2_Pct"] +
            row["L3_Pct"] / 2
        )

        if row["L1_Pct"] > 3:
            plt.text(
                l1_position,
                row["Algorithm"],
                f"{row['L1_Pct']:.1f}",
                ha="center",
                va="center"
            )

        if row["L2_Pct"] > 3:
            plt.text(
                l2_position,
                row["Algorithm"],
                f"{row['L2_Pct']:.1f}",
                ha="center",
                va="center"
            )

        if levels == 3 and row["L3_Pct"] > 3:
            plt.text(
                l3_position,
                row["Algorithm"],
                f"{row['L3_Pct']:.1f}",
                ha="center",
                va="center"
            )

    plt.xlabel(
        "Hits relative to Belady, %"
    )

    plt.ylabel(
        "Cache configuration"
    )

    if top_n is None:
        title = (
            f"All {levels}-Level Cache "
            f"Configurations — {mode}"
        )
    else:
        title = (
            f"Top {top_n} {levels}-Level Cache "
            f"Configurations — {mode}"
        )

    plt.title(title)

    plt.legend(
        title="Level"
    )

    plt.grid(
        axis="x",
        alpha=0.3
    )

    plt.tight_layout()

    if top_n is None:
        filename = (
            f"all_{levels}_level_"
            f"{mode.lower()}.png"
        )
    else:
        filename = (
            f"top_{top_n}_{levels}_level_"
            f"{mode.lower()}.png"
        )

    plt.savefig(
        os.path.join(
            OUT_DIR,
            filename
        ),
        dpi=200
    )

    plt.close()


def create_plots():
    if not os.path.exists(CSV_PATH):
        print(
            f"Error: {CSV_PATH} not found."
        )
        return

    os.makedirs(
        OUT_DIR,
        exist_ok=True
    )

    df = pd.read_csv(
        CSV_PATH
    )

    df["Algorithm"] = df[
        "Algorithm"
    ].str.replace(
        "+",
        " → ",
        regex=False
    )

    plot_single_cache(df)

    # Top 10
    plot_multi(
        df,
        levels=2,
        mode="Exclusive",
        top_n=10
    )

    plot_multi(
        df,
        levels=2,
        mode="Inclusive",
        top_n=10
    )

    plot_multi(
        df,
        levels=3,
        mode="Exclusive",
        top_n=10
    )

    plot_multi(
        df,
        levels=3,
        mode="Inclusive",
        top_n=10
    )

    # All configurations
    plot_multi(
        df,
        levels=2,
        mode="Exclusive"
    )

    plot_multi(
        df,
        levels=2,
        mode="Inclusive"
    )

    plot_multi(
        df,
        levels=3,
        mode="Exclusive"
    )

    plot_multi(
        df,
        levels=3,
        mode="Inclusive"
    )

    print(
        f"Plots successfully generated "
        f"and saved to {OUT_DIR}/"
    )


if __name__ == "__main__":
    create_plots()
