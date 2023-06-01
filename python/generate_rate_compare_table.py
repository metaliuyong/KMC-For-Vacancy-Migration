import numpy as np
import pandas as pd

k_b = 1.3806485e-23
e = 1.60217733e-19


def compute_ratio(delta_e, t):
    global k_b
    return np.exp(delta_e * e / (k_b * t))


if __name__ == "__main__":
    Delta_E = np.linspace(0.0, 2.0, num=21, endpoint=True)
    T = [300.0, 500.0, 800.0, 1000.0, 1500.0, 2000.0, 3000.0, 5000.0]

    row_size = len(Delta_E)
    column_size = len(T)

    data = np.zeros((row_size, column_size), dtype=np.float64)

    for i in range(row_size):
        for j in range(column_size):
            delta_e = Delta_E[i]
            t = T[j]
            data[i][j] = compute_ratio(delta_e, t)

    # with open("rate_compare_table.txt", "w") as f:

    data_pandas = pd.DataFrame(data, index=Delta_E, columns=T)
    data_pandas.to_excel('rate_compare_table.xls')
    print(data_pandas)
