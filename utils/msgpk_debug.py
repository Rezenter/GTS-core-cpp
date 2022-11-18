import json
import msgpack

path = 'd:/data/db/plasma/raw/'
#path = 'd:/data/db/debug/raw/'
group_count = 8
_ch_count = 2

shotn = 42500

shot_folder = '%s%05d' % (path, shotn)
FILE_EXT = 'msgpk'

for board in range(8):
    with open('%s/%s.%s' % (shot_folder, '%s' % board, FILE_EXT), 'rb') as file:
        data = msgpack.unpackb(file.read())
        for event_ind in range(len(data)):
            print(data[event_ind]['t'])
            #data[event_ind]['t'] = 3.6 + (event_ind - 1)*1000/330
    #with open('%s/%s.%s' % (shot_folder, '%s_new' % board, FILE_EXT), 'wb') as file:
    #    file.write(msgpack.packb(data))

print('Code OK')
