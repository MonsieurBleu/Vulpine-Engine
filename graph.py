#!/usr/bin/python3
import matplotlib.pyplot as plt
import pandas as pd
import sys
import argparse
import re
import os
import glob

path = ""


def getMostRecentBenchmarkFile():
    root = "build/output/"
    files = list(filter(os.path.isfile, glob.glob(root + "*.csv")))
    files.sort(key=lambda x: os.path.getmtime(x), reverse=True)
    return files[0]

    # files = [i for i in files if i.endswith(
    #     ".csv") and i.startswith("benchmark")]

    # filesAndStamps = {}

    # for i in files:
    #     m = re.search(
    #         r"(?<=benchmark_)(\d+)-(\d+)-(\d+)_(\d+)-(\d+)-(\d+)(?=\.csv)", i)

    #     if m:
    #         timestamp = (int(m.group(1)) - 1970) * 31556926
    #         timestamp += int(m.group(2)) * 2629743
    #         timestamp += int(m.group(3)) * 86400
    #         timestamp += int(m.group(4)) * 3600
    #         timestamp += int(m.group(5)) * 60
    #         timestamp += int(m.group(6))

    #         filesAndStamps[i] = timestamp

    # if (len(filesAndStamps) == 0):
    #     print("No benchmark files found")
    #     sys.exit(1)

    # mostRecent = max(filesAndStamps.values())
    # mostRecentFile = [k for k, v in filesAndStamps.items()
    #                   if v == mostRecent][0]

    # return os.path.join(root, mostRecentFile)


parser = argparse.ArgumentParser(
    description='Plot benchmark results from a csv file')

parser.add_argument('graphs', type=str, nargs='+',
                    help='The columns to graph in the format "title, x, y1, y2, ...". Optionally, you can add xLabel= and yLabel= to specify the labels for the x and y axis.')

parser.add_argument('--file', type=str, nargs='*', default=[getMostRecentBenchmarkFile()],
                    help='The csv file(s) to read the data from')

parser.add_argument('--xOffset', type=int, default=0,
                    help='The offset to add to the x axis')

parser.add_argument('--list', action='store_true',
                    help='List the columns in the file')

args = parser.parse_args()

paths = args.file

# df = pd.read_csv(path)

dfs = []
for path in paths:
    dfs.append(pd.read_csv(path))

if (args.list):
    for i in range(len(dfs)):
        print(f"File {i}: {args.file[i]}")
        print(dfs[i].columns)
    sys.exit(0)

# plot the data, with x-axis as the time and y axis as the rest of the columns, excluding the first one
# label the y axis with the column names
# print(df.columns)
# df.plot(x='time', y=df.columns[2:], title="Benchmark Results")
# plt.ylabel("App Time (s)")
# plt.xlabel("Time (ms)")
# plt.show()


def plot(dfs, argDf, title, x, *y, yLabels, xLabel=None, yLabel=None):
    f = plt.figure()
    # df.plot(x=x, y=list(y), title=title, ax=f.gca())
    series = []
    for i in range(len(y)):
        series.append(dfs[argDf[i]][y[i]][args.xOffset:])

    for i in range(len(y)):
        plt.plot(dfs[argDf[i]][x][args.xOffset:], series[i], label=yLabels[i])

    plt.title(title)
    plt.legend()

    if (yLabel):
        plt.ylabel(yLabel)
    if (xLabel):
        plt.xlabel(xLabel)

    f.show()
    return f


def getStringLikeliness(string, target):
    return sum([1 if i in target else 0 for i in string]) / max(len(string), len(target))


def getMostSimilarString(string, list):
    return max(list, key=lambda x: getStringLikeliness(string, x))


def getMostSimilarColumnOrNone(string, df):
    columns = df.columns
    mostSimilar = getMostSimilarString(string, columns)
    if (getStringLikeliness(string, mostSimilar) > 0.5):
        return mostSimilar
    return None


def existsIdenticalColumnButDifferentCase(string, df):
    columns = df.columns
    for i in columns:
        if (i.lower() == string.lower()):
            return i
    return None


for graph in args.graphs:
    arguments = graph.split(',')
    if (len(arguments) < 3):
        print("Error: Not enough arguments")
        sys.exit(1)
    title = arguments[0].strip()
    x = arguments[1].strip().split("[")[0].strip()
    y = [i.strip().split("[")[0].strip() for i in arguments[2:]]
    labelX = None
    labelY = None
    toRemove = []
    for i in y:
        iLower = i.lower()
        if re.match("(label)?x(label)?=", iLower):
            labelX = i.split('=')[1]
            toRemove.append(i)
        elif re.match("(label)?y(label)?=", iLower):
            labelY = i.split('=')[1]
            toRemove.append(i)

    for i in toRemove:
        y.remove(i)

    yDfs = []
    for i in arguments[2:]:
        if ("[" in i):
            yDfs.append(int(i.split("[")[1].split("]")[0]))
        else:
            yDfs.append(0)

    # check if the columns are valid
    for yDf in yDfs:
        if (x not in dfs[yDf].columns):
            identicalColumn = existsIdenticalColumnButDifferentCase(
                x, dfs[yDf])
            if (identicalColumn):
                x = identicalColumn
            else:
                print(f"Error: {x} is not a valid column")
                mostSimilar = getMostSimilarColumnOrNone(x, dfs[yDf])
                if (mostSimilar):
                    print(f"Did you mean {mostSimilar}?")
                sys.exit(1)

    for i in range(len(y)):
        if (y[i] not in dfs[yDfs[i]].columns):
            identicalColumn = existsIdenticalColumnButDifferentCase(
                y[i], dfs[yDfs[i]])
            if (identicalColumn):
                y[y.index(y[i])] = identicalColumn
            else:
                print(f"Error: {y[i]} is not a valid column")
                mostSimilar = getMostSimilarColumnOrNone(y[i], dfs[yDfs[i]])
                if (mostSimilar):
                    print(f"Did you mean {mostSimilar}?")
                sys.exit(1)

    dfIndices = yDfs

    if (len(args.file) == 1):
        ylabels = [f"{y[i]}" for i in range(len(y))]
    else:
        ylabels = [
            f"{y[i]} (fichier {os.path.split(args.file[dfIndices[i]])[-1]})" for i in range(len(y))]
    f = plot(dfs, dfIndices, title, x, *y,
             yLabels=ylabels, xLabel=labelX, yLabel=labelY)


while (any([plt.fignum_exists(i) for i in plt.get_fignums()])):
    plt.pause(0.1)
