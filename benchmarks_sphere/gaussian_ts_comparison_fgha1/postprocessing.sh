#! /bin/bash

./pp_plot_csv.py reference output_rk4_ts0.0001/prog_h_t00000000007.00000000.csv output_*/prog_h_t00000000007.00000000.csv

for i in output_*/prog_h_t00000000007.00000000.png; do
	cp "$i" "result_${i//\//__}"
done