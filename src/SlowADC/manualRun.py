import subprocess

shotn = '42700'
in_path = 'd:/data/db/plasma/slow/raw/'
out_path = 'd:/data/db/plasma/slow/sht/'
config_file = "d:/data/db/config/2022.12.19_1.6J_slow.json"

proc = subprocess.run(['SlowADC_to_sht.exe', shotn, in_path, out_path, config_file])
if proc.returncode != 0:
    print('FUCK')

print('Code ok')
