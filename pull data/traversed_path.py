import argparse
import configparser
from itertools import groupby

import psycopg2
import sys

data = ["471876-463980", "463980-463981", "463981-438759", "438759-438760", "42670-515687", "515687-240205",
        "240205-22457", "22458-22457", "434076-451381", "451381-451380", "451381-640019", "640019-291718",
        "291718-226164", "226164-640020"]

next_id = 0
segments = dict()


def get_next_id():
    global next_id
    result = "#" + str(next_id)
    next_id += 1
    return result


def func(trip: tuple):
    global segments
    if trip[1] not in segments:
        short_name = get_next_id()
        segments[trip[1]] = short_name
        return trip[0], short_name
    else:
        return trip[0], segments.get(trip[1])


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Download frequently traversed paths and report their properties')
    parser.add_argument('--ini', required=True,
                        help='The configuration file, e.g., transfer.ini')
    parser.add_argument('-n', type=str, required=False, help='Database name')
    parser.add_argument('-i', type=str, required=False, help='Database IP')
    parser.add_argument('-po', type=str, required=False, help='Database port')
    parser.add_argument('-u', type=str, required=False, help='Username')
    parser.add_argument('-pa', type=str, required=False, help='password')
    parser.add_argument('-c', type=str, required=False, help='frequent count')

    args = parser.parse_args()
    config = configparser.ConfigParser()
    # The default values will be used if some of the parameters are empty/None
    config.read(args.ini)
    if args.n is None:
        db_name = config.get('DEFAULT', 'db_name')
    else:
        db_name = args.n
    if args.i is None:
        ip = config.get('DEFAULT', 'ip')
    else:
        ip = args.i
    if args.po is None:
        port = config.get('DEFAULT', 'port')
    else:
        port = args.po
    if args.u is None:
        user = config.get('DEFAULT', 'username')
    else:
        user = args.u
    if args.pa is None:
        pw = config.get('DEFAULT', 'password')
    else:
        pw = args.pa
    if args.c is None:
        count = config.get('DEFAULT', 'count')
    else:
        count = args.c
    tp = config.get('DEFAULT', 'tp')
    filename = config.get('DEFAULT', 'output_file')
    output = open(filename, "w")

    print("frequently traversed count is: " + str(count))
    conn = psycopg2.connect(database="au_db", user=user, password=pw, host=ip, port=port)
    cur = conn.cursor()
    cur.execute("SELECT trip_id, seg_id "
                "FROM trips")
    # data = cur.fetchall()
    data = cur.fetchmany(25)
    sorted_data = sorted(data, key=lambda x: x[0])
    del data

    # map seg_ids to shorter names
    sorted_data = [func(item) for item in sorted_data]

    groups = []
    unique_keys = []
    for k, g in groupby(sorted_data, lambda x: x[0]):
        groups.append(list(g))  # Store group iterator as a list
        unique_keys.append(k)

    concatenated = []
    for group in groups:
        concatenated.append((group[0][0], ("".join([item[1] for item in group]))))

    traversals = dict()
    concatenated.append((-1, concatenated[0][1]))
    for i in concatenated:
        for j in concatenated:
            if i[1] in j[1]:
                if i[0] in traversals:
                    traversals[i[1]] += j[1].count(i[1])
                else:
                    traversals[i[1]] = j[1].count(i[1])
    print("Done")
    pass
