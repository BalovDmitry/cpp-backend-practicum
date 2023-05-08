
import argparse
import subprocess
import time
import random
import shlex
import os

RANDOM_LIMIT = 1000
SEED = 123456789
random.seed(SEED)

AMMUNITION = [
    'localhost:8080/api/v1/maps/map1',
    'localhost:8080/api/v1/maps'
]

SHOOT_COUNT = 100
COOLDOWN = 0.1


def start_server():
    parser = argparse.ArgumentParser()
    parser.add_argument('server', type=str)
    return parser.parse_args().server


def run(command, output=None):
    process = subprocess.Popen(shlex.split(command), stdout=output, stderr=subprocess.DEVNULL)
    return process


def stop(process, wait=False):
    if process.poll() is None and wait:
        process.wait()
    process.terminate()

def shoot(ammo):
    hit = run('curl ' + ammo, output=subprocess.DEVNULL)
    time.sleep(COOLDOWN)
    stop(hit, wait=True)


def make_shots():
    for _ in range(SHOOT_COUNT):
        ammo_number = random.randrange(RANDOM_LIMIT) % len(AMMUNITION)
        shoot(AMMUNITION[ammo_number])
    print('Shooting complete')

def record(process, output=None):
	pid = process.pid
	print("Start recording")
	list = ['perf', 'record', '-p', pid, '-o', 'perf.data']
	command = ' '.join(map(str, list))
	process = run(command, None)
	#process = subprocess.Popen(shlex.split(command), stdout=output, stderr=subprocess.DEVNULL)
	return process

def make_result():
	print('Make result')
	command=f'sudo perf script | ./FlameGraph/stackcollapse-perf.pl | ./FlameGraph/flamegraph.pl -i perf.data  > graph.svg'
	os.system(command)	
	#process=run(command, output=subprocess.DEVNULL)
	#return process

server = run(start_server())
record(server)
make_shots()
stop(server)
time.sleep(5)
print('Job done')
make_result()
time.sleep(2)
#stop(result)
