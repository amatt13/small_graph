"""
    Created by Anders Lykke Matthiassen
    amatt13@student.aau.dk OR
    anders.matthiassen@gmail.com
"""

import argparse
import configparser
import pickle
from itertools import groupby
import psycopg2

FIRST_ENTRY_VALUE = "NA"

# Short names for seg_id
next_id = -1
# All recorded segments (segment = edge-edge)
segments = dict()


def get_next_id():
    """
    ID producer
    :return: A new ID i the format '#[number]'
    """
    global next_id
    next_id += 1
    return next_id


def shorten(trip: tuple):
    """
    Add segment to segments dict and rename the seg_id,
    or return the short name if already seen
    :param trip: trip_id, seg_id
    :return: trip_id, short_name
    """
    global segments
    if trip[1] not in segments:
        short_name = get_next_id()
        segments[trip[1]] = short_name
        return trip[0], short_name
    else:
        return trip[0], segments.get(trip[1])


frequent_paths = []


def fetch_data(save_to_file: bool, all_data: bool = True):
    """
    Read input arguments and get data from DB
    :param save_to_file: should the fetched data be saved in a file?
    :param all_data: Use all of the data from the DB, or only a subset (debugging)
    :return: All of the trips; The minimum traverse count; The max cardinality
    """
    parser = argparse.ArgumentParser(description='Download frequently traversed paths and report their properties')
    parser.add_argument('--ini', required=True, help='The configuration file, e.g., transfer.ini')
    parser.add_argument('-n', type=str, required=False, help='Database name')
    parser.add_argument('-i', type=str, required=False, help='Database IP')
    parser.add_argument('-po', type=str, required=False, help='Database port')
    parser.add_argument('-u', type=str, required=False, help='Username')
    parser.add_argument('-pa', type=str, required=False, help='password')
    parser.add_argument('-fc', type=str, required=False, help='frequent count')
    parser.add_argument('-mc', type=str, required=False, help='max cardinality')

    # argument handling / read ini file
    args = parser.parse_args()
    config = configparser.ConfigParser()
    config.read(args.ini)
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
    if args.fc is None:
        traverse_count = config.get('DEFAULT', 'count')
    else:
        traverse_count = args.fc
    if args.mc is None:
        cardinality = config.get('DEFAULT', 'cardinality')
    else:
        cardinality = args.mc

    print("Frequently traversed count is: {0}\nMax cardinality is: {1}".format(traverse_count, cardinality))
    traverse_count = int(traverse_count)  # change type to int...
    cardinality = int(cardinality)

    conn = psycopg2.connect(database="au_db", user=user, password=pw, host=ip, port=port)
    cur = conn.cursor()
    print("Fetching...")
    if all_data:
        cur.execute("SELECT m.trip_id, m.seg_id "
                    "FROM ( "
                    "   SELECT trip_id "
                    "   FROM trips_correction "
                    "   GROUP BY trip_id"
                    "   ORDER BY trip_id"
                    ") t "
                    "JOIN trips m "
                    "ON m.trip_id = t.trip_id")
    else:
        cur.execute("SELECT m.trip_id, m.seg_id "
                    "FROM ( "
                    "   SELECT trip_id "
                    "   FROM trips_correction "
                    "   WHERE trip_id >= 295000"
                    "   AND trip_id <=   300000"
                    "   GROUP BY trip_id"
                    "   ORDER BY trip_id"
                    ") t "
                    "JOIN trips m "
                    "ON m.trip_id = t.trip_id")
    data = cur.fetchall()
    print("Data collected...\nMapping...")

    # Format sorted_data into a list that contains the trip_id and then all of the visited seg_ids
    # trip0, (seg1, seg2, seg3...)
    groups = []
    for k, g in groupby(data, lambda x: x[0]):
        groups.append(list(g))

    # Fix mistakes in the DB
    # Some trips have "teleporting" vehicles
    # Whenever a vehicle teleports, a new trip is created
    for entry in groups:
        if len(entry) > 1:
            prev_dest = FIRST_ENTRY_VALUE
            i = 0
            for edge in reversed(entry):
                split = edge[1].split('-')
                start = split[0]
                if start != prev_dest and prev_dest != FIRST_ENTRY_VALUE:
                    groups.append(entry[-i:])
                    groups.append(entry[:-i])
                    groups.remove(entry)
                    entry = entry[:-i]
                    i = 0
                prev_dest = split[1]
                i += 1

    # map seg_ids to shorter names (easier debugging)
    # sorted_data = [shorten(item) for item in data]

    # Concatenate the seg_id list into a single string
    trips = []
    for group in groups:
        trips.append((group[0][0], tuple([item[1] for item in group])))

    # Save everything
    if save_to_file:
        print("Saving all_data")
        with open('all_data', 'wb') as f:
            pickle.dump(trips, f, pickle.HIGHEST_PROTOCOL)

    return trips, traverse_count, cardinality


def create_frequent_paths_1(trips: list, min_traversal: int):
    """
    Count and save the number of times that a path have been traversed
    :param trips: a list of trips
    :param min_traversal: minimum number of times that a trip has be traversed in order to be added to the result
    :return: void (result is saved to the global list 'frequent_paths')
    """
    frequent_paths.append(list())
    edge_list = []
    edge_counter = dict()
    trips_length = len(trips)
    count = 1

    # Count the number of times a trip have been traversed
    print("Total trips count: " + str(trips_length))
    for trip in trips:
        if count % 1000 == 0:
            print(str(count) + "/" + str(trips_length))  # progress update: mod 1000
        elif count == trips_length:
            print(str(count) + "/" + str(trips_length))  # progress update: done(last)
        count += 1
        for edge in trip[1]:
            if edge in edge_list:
                edge_counter[edge] = edge_counter.get(edge) + 1
            else:
                edge_list.append(edge)
                edge_counter[edge] = 1

    # Save those that meets the min_traversal variable requirement
    for edge in edge_counter:
        if edge_counter[edge] >= min_traversal:
            frequent_paths[0].append((edge,))


def hot_paths(trips: list, min_traversal: int, cardinality: int):
    """
    Find and save hot paths
    :param trips: A list of trips
    :param min_traversal: minimum number of times that a trip has be traversed in order to be added to the result
    :param cardinality: The maximum number of edges to examine at the time (start at 2)
    :return: void (results are saved in multiple files)
    """
    trip_counter = {}
    y = 2  # Start cardinality
    while y <= cardinality:
        print("Trips size: " + str(len(trips)))
        print("frequent_paths_" + str(y-2) + " size: " + str(len(frequent_paths[y-2])))
        print("Cardinality: " + str(y))
        frequent_paths.append(list())
        for trip in trips:
            length = len(trip[1])
            if length < y:  # Only examine those trips which has at least y edges
                # If it has less than y edges, it won't have y+1 edges either and should not be considered in the next iteration
                trips.remove(trip)
            else:
                i = 0
                while i < length - y + 1:
                    a = trip[1][i: i + y - 1]
                    b = trip[1][i + 1: i + 1 + y - 1]
                    if a in frequent_paths[y-2] and b in frequent_paths[y-2]:
                        c = trip[1][i: i + 1 + y - 1]
                        if trip_counter.get(c) is None:
                            trip_counter[c] = 1, [trip[0]]
                        else:
                            trip_counter[c] = trip_counter[c][0] + 1, trip_counter[c][1] + [trip[0]]
                    i += 1
        write_frequent(trip_counter, min_traversal, y)  # save trips in file
        trip_counter.clear()
        y += 1
    # Done, now go save everything (debugging)
    # with open('create_frequent_paths', 'wb') as f:
    #     pickle.dump(frequent_paths, f, pickle.HIGHEST_PROTOCOL)


def write_frequent(trip_counter: dict, min_traversal: int, cardinality: int):
    """
    Save trips in file and add them to the frequent_paths list
    :param trip_counter: dict that contains the trips and the number of times they have been traversed
    :param min_traversal: minimum number of times that a trip has be traversed
    :param cardinality: current cardinality
    :return: void (result is saved in a file 'frequent_paths_<cardinality>')
    """
    with open('frequent_paths_{0}'.format(str(cardinality)), 'w') as f:
        for path in trip_counter:
            if trip_counter.get(path)[0] > min_traversal:
                frequent_paths[cardinality - 1].append(path)
                f.write("{0};{1};{{{2}}};\n".format(  # read more about the format in the documentation document
                    ", ".join(path),
                    str(trip_counter.get(path)[0]),
                    ", ".join(str(trip_id) for trip_id in trip_counter.get(path)[1]))
                )


def start():
    """
    This is just a function that can be called from other files if imported
    Calling main is just as valid
    """
    trips, traverse_count, cardinality = fetch_data(save_to_file=True, all_data=True)
    create_frequent_paths_1(trips=trips, min_traversal=traverse_count)
    hot_paths(trips=trips, min_traversal=traverse_count, cardinality=cardinality)
    print("Done")


if __name__ == '__main__':
    start()
