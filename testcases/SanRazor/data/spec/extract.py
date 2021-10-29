import csv
import argparse

parser = argparse.ArgumentParser(description='Run privacy.py.')
parser.add_argument('--spec_path', help="work directory of spec", type=str, default="~/workspace")
parser.add_argument('--setup', help="SR_asan_L0/SR_asan_L1/SR_asan_L2/SR_ubsan_L0/SR_ubsan_L1/SR_ubsan_L2", type=str, default="SR_asan_L2")
args = parser.parse_args()
setup = args.setup
spec_path = args.spec_path

BENCHMARKS = [   
    "401.bzip2",
    "429.mcf",
    "445.gobmk",
    "456.hmmer",
    "458.sjeng",
    "462.libquantum",
    "433.milc",
    "470.lbm",
    "482.sphinx3",
    "444.namd",
    "453.povray"
]

for benchmark in BENCHMARKS:
    check_stat_file = "{}/SPEC_CPU2006v1.0/benchspec/CPU2006/{}/run/build_peak_{}.0001/Cov/check.txt".format(spec_path, benchmark, setup)
    try:
        with open(check_stat_file, 'r') as csv_file:
            csv_reader = csv.reader(csv_file, delimiter=' ')
            line_count = 0
            num_sc = 0
            num_sc_remain = 0
            cost_sc = 0
            cost_sc_remain = 0
            for row in csv_reader:
                if line_count % 2 == 0:
                    num_sc += int(row[1])
                    num_sc_remain += int(row[3])
                    cost_sc += int(row[7])
                    cost_sc_remain += int(row[8])
                line_count += 1
            print("[INFO] Benchmark: {}, setup: {}".format(benchmark, setup))
            print("[INFO] M1: {}".format((num_sc - num_sc_remain) / num_sc))
            print("[INFO] M2: {}".format((cost_sc - cost_sc_remain) / cost_sc))
    except:
        print('[ERR] No results for benchmark: {}'.format(benchmark))
