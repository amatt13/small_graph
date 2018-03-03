import argparse
import configparser
import pickle
from itertools import groupby
import psycopg2

# Debug data
# (trip_id, seg_id)
data = [('1', 'a-b'), ('1', 'b-c'), ('1', 'c-d'), ('1', 'c-d'), ('1', 'b-c'), ('1', 'c-d'),
        ('2', 'b-c'), ('2', 'c-d'),
        ('3', 'a-b'), ('3', 'a-b'), ('3', 'a-b'),
        ('4', 'a-b'),
        ('5', 'b-c'), ('5', 'c-d'), ('5', 'c-d')]

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


# def hot_paths_old(trips):
#     # Dict where the key is the concatenated seg_ids and the value is the trip_id and the number of times it have been
#     # traversed
#     traversals = dict()
#     for trip in trips:
#         traversals[trip[1]] = trip[0], 0
#
#     # Count and save the number of times that a path have been traversed
#     traversals_length = len(traversals)
#     count = 1
#     print("Total count: " + str(traversals_length))
#     for path in traversals:
#         if count % 100 == 0:
#             print(str(count) + "/" + str(traversals_length))  # progress update: mod 100
#         elif count == traversals_length:
#             print(str(count) + "/" + str(traversals_length))  # progress update: done(last)
#         count += 1
#         for trip in trips:
#             occurrences = len(re.findall(path, trip[1], overlapped=True))
#             if occurrences >= 1:
#                 traversals[path] = traversals.get(path)[0], traversals.get(path)[1] + occurrences
#
#     results = []
#     for path in traversals:
#         value = traversals.get(path)
#         if value[1] >= traverse_count:
#             results.append((value[0], value[1]))
#
#     print("Done")
#     print("Traversal Counts")
#     for result in results:
#         print(str(result[0]) + ":" + str(result[1]))


frequent_paths = []


def create_frequent_paths_1(trips: list, min_traversal: int):
    # Count and save the number of times that a path have been traversed
    frequent_paths.append(list())
    edge_list = []
    edge_counter = dict()
    trips_length = len(trips)
    count = 1
    print("Total trips count: " + str(trips_length))
    for trip in trips:
        if count % 1000 == 0:
            print(str(count) + "/" + str(trips_length))  # progress update: mod 100
        elif count == trips_length:
            print(str(count) + "/" + str(trips_length))  # progress update: done(last)
        count += 1
        for edge in trip[1]:
            if edge in edge_list:
                edge_counter[edge] = edge_counter.get(edge) + 1
            else:
                edge_list.append(edge)
                edge_counter[edge] = 1
    for edge in edge_counter:
        if edge_counter[edge] >= min_traversal:
            frequent_paths[0].append((edge,))


def hot_paths(trips: list, x: int, cardinality: int):
    trip_counter = {}
    y = 2
    while y <= cardinality:
        print("Trips size: " + str(len(trips)))
        print("frequent_paths_" + str(y-2) + " size: " + str(len(frequent_paths[y-2])))
        print("Cardinality: " + str(y))
        frequent_paths.append(list())
        for trip in trips:
            length = len(trip[1])
            if length < y:
                trips.remove(trip)
            else:
                i = 0
                while i < length - y + 1:
                    a = trip[1][i: i + y - 1]
                    b = trip[1][i + 1: i + 1 + y - 1]
                    if a in frequent_paths[y-2] and b in frequent_paths[y-2]:
                        c = trip[1][i: i + 1 + y - 1]
                        if trip_counter.get(c) is None:
                            trip_counter[c] = 1
                        else:
                            trip_counter[c] = trip_counter[c] + 1
                    i += 1
        for path in trip_counter:
            if trip_counter.get(path) > x:
                frequent_paths[y-1].append(path)
        trip_counter.clear()
        y += 1

    # Done, now go save everything
    with open('create_frequent_paths', 'wb') as f:
        pickle.dump(frequent_paths, f, pickle.HIGHEST_PROTOCOL)


def db_stuff():
    parser = argparse.ArgumentParser(description='Download frequently traversed paths and report their properties')
    parser.add_argument('--ini', required=True,
                        help='The configuration file, e.g., transfer.ini')
    parser.add_argument('-n', type=str, required=False, help='Database name')
    parser.add_argument('-i', type=str, required=False, help='Database IP')
    parser.add_argument('-po', type=str, required=False, help='Database port')
    parser.add_argument('-u', type=str, required=False, help='Username')
    parser.add_argument('-pa', type=str, required=False, help='password')
    parser.add_argument('-c', type=str, required=False, help='frequent count')

    # argument handling / read ini file
    args = parser.parse_args()
    config = configparser.ConfigParser()
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
        traverse_count = config.get('DEFAULT', 'count')
    else:
        traverse_count = args.c

    tp = config.get('DEFAULT', 'tp')
    filename = config.get('DEFAULT', 'output_file')
    output = open(filename, "w")

    print("Frequently traversed count is: " + traverse_count)
    traverse_count = int(traverse_count)  # change type to int...
    conn = psycopg2.connect(database="au_db", user=user, password=pw, host=ip, port=port)
    cur = conn.cursor()

    print("Fetching...")
    # cur.execute("SELECT m.trip_id, m.seg_id "
    #             "FROM ( "
    #             "   SELECT trip_id "
    #             "   FROM trips "
    #             "   WHERE trip_id >= 251627"
    #             "   AND trip_id <= 3000000"
    #             "   GROUP BY trip_id"
    #             ") t "
    #             "JOIN trips m "
    #             "ON m.trip_id = t.trip_id")

    cur.execute("SELECT m.trip_id, m.seg_id "
               "FROM ( "
               "   SELECT trip_id "
               "   FROM trips "
               "   GROUP BY trip_id"
               ") t "
               "JOIN trips m "
               "ON m.trip_id = t.trip_id")
    data = cur.fetchall()
    print("Data collected...\nMapping...")

    # map seg_ids to shorter names
    sorted_data = [shorten(item) for item in data]

    # Format sorted_data into a list that contains the trip_id and then all of the visited seg_ids
    # trip0, (seg1, seg2, seg3...)
    groups = []
    for k, g in groupby(sorted_data, lambda x: x[0]):
        groups.append(list(g))

    # Concatenate the seg_id list into a single string
    trips = []
    for group in groups:
        trips.append((group[0][0], tuple([item[1] for item in group])))

    # Save everything
    with open('all_data', 'wb') as f:
        pickle.dump(trips, f, pickle.HIGHEST_PROTOCOL)

    return trips, traverse_count


if __name__ == '__main__':
    trips, traverse_count = db_stuff()
    #with open('short', 'rb') as f:
    #    X = pickle.load(f)
    #sorted_data = pickle.load(open("data", "rb"))

    create_frequent_paths_1(trips, traverse_count)
    hot_paths(trips, traverse_count, 10)
    pass
