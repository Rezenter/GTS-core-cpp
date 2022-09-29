import json
import msgpack

path = 'd:/data/db/debug/raw/'
group_count = 8
_ch_count = 2

DB_PATH = 'd:/data/db/'
DEBUG_SHOTS = 'debug/'
shotn = 565

print(shotn)
shot_folder = '%s%05d' % (path, shotn)
FILE_EXT = 'msgpk'

with open('%s/%s.%s' % (shot_folder, '0', FILE_EXT), 'rb') as file:
    data = msgpack.unpackb(file.read())
    with open('tmp.json', 'w') as out_file:
        json.dump(data, out_file)

print('Code OK')
