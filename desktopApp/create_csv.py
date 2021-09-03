import csv
import json
from os import listdir, makedirs
from os.path import isfile, join, realpath, dirname, isdir, basename


def create_csv(logs_folder: str, dest_file):
    cwd = dirname(realpath(__file__))
    logs_folder = logs_folder[1:] if logs_folder[0] == '/' else logs_folder
    logs_folder = join(cwd, logs_folder)
    dest_folder = join(cwd, dirname(dest_file))
    dest_file = join(dest_folder, basename(dest_file))
    if not isdir(dest_folder):
        makedirs(dest_folder)

    log_files = [join(logs_folder, f) for f in listdir(logs_folder) if isfile(join(logs_folder, f))]
    
    with open(dest_file, 'w') as csvfile:
        quant_sensors = -1
        csv_writer = csv.writer(csvfile, delimiter=',')
        for file in log_files:
            with open(file, 'r') as f:
                for line in f.readlines():
                    try:
                        log = json.loads(line)
                        if 'time' in log and 'soil' in log and 'hum' in log and 'temp' in log:
                            if quant_sensors == -1:
                                quant_sensors = len(log['soil'])
                                l1 = ['time', 'soil'] + ['']*max(0, quant_sensors-1) + ['hum', 'temp']
                                csv_writer.writerow(l1)
                            l2 = [log['time']] + [m for m in log['soil']] + [log['hum'], log['temp']]
                            csv_writer.writerow(l2)
                    except:
                        print('error parsing log line')