import json
import msgpack

#path = 'd:/data/db/plasma/raw/'
path = 'd:/data/db/debug/raw/'
group_count = 8
_ch_count = 2

shotn = 676

shot_folder = '%s%05d' % (path, shotn)
FILE_EXT = 'msgpk'

board = 1
with open('%s/%s.%s' % (shot_folder, '%s' % board, FILE_EXT), 'rb') as file:
    data = msgpack.unpackb(file.read())
    for event_ind in range(len(data)):
        line = '%d, %d, ' % (data[event_ind]['t'], data[event_ind]['DAC1'])
        ph_el = 0
        for ch in range(5):
            ph_el += data[event_ind]['ph_el'][ch + 11]
            line += '%d, ' % data[event_ind]['ph_el'][ch + 11]
        print('%s%d' % (line, ph_el))
    print('')
    for cell in range(1024):
        print(data[1]['ch'][11+2][cell])
        #data[event_ind]['t'] = 3.6 + (event_ind - 1)*1000/330
#with open('%s/%s.%s' % (shot_folder, '%s_new' % board, FILE_EXT), 'wb') as file:
#    file.write(msgpack.packb(data))

print('Code ok')
