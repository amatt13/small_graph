import argparse
import configparser
import psycopg2
import sys
from enum import Enum

class GraphType(Enum):
    full = "0"
    denmark = "1"
    north_jutland = "2"
    aalborg = "3"

# Aalborg
aal_north = 57.079702
aal_south = 57.005367
aal_west = 09.859484
aal_east = 10.032176

# North Jutland
northj_north = 57.778230
northj_south = 56.539097
northj_west = 8.147842
northj_east = 11.432754


def get_location_enum(l: str):
    if l == "full":
        return GraphType.full
    elif l == "denmark":
        return GraphType.denmark
    elif l == "north_jutland" or l == "jutland" or l == "northJutland":
        return GraphType.north_jutland
    elif l == "aalborg":
        return GraphType.aalborg
    else:
        sys.exit("Invalid location: " + l)


def write_edge_information(data):
    i = 0
    query_column_count = len(data)
    if query_column_count > 0:
        output.write("{\n")
        while i < query_column_count:
            if len(str(data[i][0].hour)) <= 1:
                res_hour = "0" + str(data[i][0].hour)
            else:
                res_hour = str(data[i][0].hour)

            if len(str(data[i][0].minute)) <= 1:
                res_minute = "0" + str(data[i][0].minute)
            else:
                res_minute = str(data[i][0].minute)

            if len(str(data[i][0].second)) <= 1:
                res_second = "0" + str(data[i][0].second)
            else:
                res_second = str(data[i][0].second)

            hist = str(location.value + " " + str(data[i][3])) + " " + str(res_hour) + ":" + str(
                res_minute) + ":" + str(res_second) + "\n"
            output.write(hist)
            i += 1
        output.write("}\n")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Download and manipulate data')
    parser.add_argument('--ini', required=True,
                        help='The configuration file, e.g., transfer.ini')
    parser.add_argument('-n', type=str, required=False, help='Database name')
    parser.add_argument('-i', type=str, required=False, help='Database IP')
    parser.add_argument('-po', type=str, required=False, help='Database port')
    parser.add_argument('-u', type=str, required=False, help='Username')
    parser.add_argument('-pa', type=str, required=False, help='password')
    parser.add_argument('-l', type=str, required=False, help='location')
    print("Started")

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
    if args.l is None:
        pw = config.get('DEFAULT', 'location')
    else:
        pw = args.pa
    tp = config.get('DEFAULT', 'tp')
    filename = config.get('DEFAULT', 'output_file')
    output = open(filename, "w")
    location = get_location_enum(args.l)

    conn = psycopg2.connect(database="au_db", user=user, password=pw, host=ip, port=port)
    cur = conn.cursor()
    cur.execute("SELECT max(id) FROM road_network")
    max_id = cur.fetchone()
    cur.execute("SELECT min(id) FROM road_network")
    min_id = cur.fetchone()
    cur.execute("SELECT max(source_vid) from road_network")
    max_source = cur.fetchone()
    cur.execute("SELECT max(target_vid) from road_network")
    max_target = cur.fetchone()
    highest_node = max(max_target[0], max_source[0])

    counter = min_id[0]
    max_count = max_id[0]
    print(counter)
    print(max_count)
    print(counter)

    # x->y AND y->x exist in the road network
    cur.execute("SELECT DISTINCT(r1.id) "
                "FROM road_network r1, road_network r2 "
                "WHERE r1.source_vid = r2.target_vid "
                "AND r2.source_vid = r1.target_vid")
    circular_id = cur.fetchall()

    # LOOPS TO BE IGNORED:
    cur.execute("SELECT DISTINCT(r1.id) "
                "FROM road_network AS r1 "
                "WHERE r1.source_vid = r1.target_vid")
    loop_ids = cur.fetchall()

    # PARALLEL DUPLICATES TO BE MERGED:

    # ID FOR PARALLEL EDGES:
    cur.execute("SELECT DISTINCT t1.id  "
                "FROM road_network t1, road_network t2, segment_type "
                "WHERE t1.source_vid = t2.source_vid "
                "AND t1.target_vid = t2.target_vid "
                "AND t1.id != t2.id "
                "AND segment_type.seg_id_old = t1.seg_id_old "
                "ORDER BY t1.id")

    parallel_duplicates_id = cur.fetchall()

    # ID and SEGID and avgspeed FOR PARALLEL EDGES ( no duplicates)
    cur.execute("SELECT DISTINCT ON (t1.seg_id) t1.source_vid, t1.target_vid, t1.length, "
                "(CEILING(segment_type.length/segment_type.actualavgspeed*3600)), t1.id, t1.seg_id  "
                "FROM road_network t1, road_network t2, segment_type "
                "WHERE t1.source_vid = t2.source_vid "
                "AND t1.target_vid = t2.target_vid "
                "AND t1.id != t2.id "
                "AND segment_type.seg_id_old = t1.seg_id_old ")

    parallel_duplicates = cur.fetchall()

    # Inside bounds
    print("start")
    cur.execute("SELECT DISTINCT(r.id) "
                "FROM road_network r, vertices v "
                "WHERE r.source_vid = v.id "
                "AND v.xpos <= %s "
                "AND v.xpos >= %s "
                "AND v.ypos <= %s "
                "AND v.ypos >= %s", (aal_east, aal_west, aal_north, aal_south))

    inside_bounds_ids = cur.fetchall()

    ignore_list = [i[0] for i in loop_ids]  # we ignore these completely
    ignore_list += [i[0] for i in parallel_duplicates_id]  # These are handled separately
    # ignore_list += [i[0] for i in outside_bounds_ids]  # ignore these as well
    ignore_list.sort()
    ignore_list = list(set(ignore_list))

    res = []
    for elem in inside_bounds_ids:
        res.append(elem[0])

    complete = list(set(res).difference(ignore_list))

    while counter < max_count:
        print(counter)
        if counter not in complete:
            counter += 1
            continue

        # Query for measurements from x->y
        cur.execute(
            "SELECT source_vid, target_vid, (CEILING(segment_type.length/segment_type.actualavgspeed*3600)) "
            "FROM road_network, segment_type "
            "WHERE road_network.id = %s "
            "AND segment_type.seg_id_old = road_network.seg_id_old",
            (counter,))
        try:
            source_target = cur.fetchone()  # source_vid, target_vid, cost
        except psycog2.ProgrammingError:
            continue
        result_string_a = "# " + str(counter) + " " + str(source_target[0]) + " " + str(source_target[1]) \
                          + " " + str(source_target[2]) + "\n"
        output.write(result_string_a)


        cur.execute(
            "SELECT trips.start_time, trips.end_time, trips.seg_id, trips.travel_time  "
            "FROM trips, road_network "
            "WHERE road_network.id = %s "
            "AND trips.seg_id_old = road_network.seg_id_old", (counter,))
        result_a = cur.fetchall()
        write_edge_information(result_a)
        # Reverse query, ask for measurements from y->x
        if counter in circular_id:
            counter += 1
            continue
        highest_node += 1
        result_string_b = "# " + str(highest_node) + " " + str(source_target[1]) + " " + str(
            source_target[0]) + " " + str(source_target[2]) + "\n"
        output.write(result_string_b)
        seg_id = str(source_target[1]) + "-" + str(source_target[0])  # target-source


        cur.execute(
            "SELECT trips.start_time, trips.end_time, trips.seg_id, trips.travel_time "
            "FROM trips "
            "WHERE seg_id = %s",
            (seg_id,))
        result_b = cur.fetchall()
        write_edge_information(result_b)
        counter += 1
        print(counter)
    output.flush()

    # Handle parallel duplicates. Ignore all but one.
    for n in parallel_duplicates:
        result = "# " + str(n[4]) + " " + str(n[0]) + " " + str(n[1]) + " " + str(n[3]) + "\n"
        output.write(result)
        cur.execute(
            "SELECT trips.start_time, trips.end_time, trips.seg_id, trips.travel_time  "
            "FROM trips, road_network "
            "WHERE road_network.source_vid = %s AND road_network.target_vid = %s AND road_network.length = %s"
            "AND trips.seg_id_old = road_network.seg_id_old", (n[0], n[1], n[2]))
        result = cur.fetchall()
        write_edge_information(result)
        # Reversed, see if there are some measurements for the other way.

        highest_node += 1
        print(highest_node)
        result = "# " + str(highest_node) + " " + str(n[1]) + " " + str(n[0]) + " " + str(n[3]) + "\n"
        output.write(result)

        seg_id = str(n[1]) + "-" + str(n[0])  # target-source
        cur.execute(
            "SELECT trips.start_time, trips.end_time, trips.seg_id, trips.travel_time FROM trips WHERE seg_id = %s",
            (seg_id,))
        result = cur.fetchall()
        write_edge_information(result)
    output.flush()
    cur.close()
