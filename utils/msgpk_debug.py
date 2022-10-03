import json
import msgpack

path = 'd:/data/db/plasma/raw/'
group_count = 8
_ch_count = 2

shotn = 42339

shot_folder = '%s%05d' % (path, shotn)
FILE_EXT = 'msgpk'

with open('%s/%s.%s' % (shot_folder, '1', FILE_EXT), 'rb') as file:
    data = msgpack.unpackb(file.read())
    #with open('tmp.json', 'w') as out_file:
    #    json.dump(data, out_file)
    for event in data:
        print(event['DAC1'], event['ph_el'][11: 15])

print('Code OK')
