#/bin/bash
#/*
# *
# * Bruno Ferrero n.USP: 3690142  Curso: BCC
# * Rodrigo Alves n.USP 6800149   Curso: BCC
# *
# * Data: Out/2017
# *
# */

perf stat -r 30 ./corrida 250 6 80  >> logs/log_06c_080v.txt 2>&1
perf stat -r 30 ./corrida 250 12 80 >> logs/log_12c_080v.txt 2>&1
perf stat -r 30 ./corrida 250 18 80 >> logs/log_18c_080v.txt 2>&1
perf stat -r 30 ./corrida 250 6 160 >> logs/log_06c_160v.txt 2>&1
perf stat -r 30 ./corrida 250 6 240 >> logs/log_06c_240v.txt 2>&1
perf stat -r 30 ./corrida 250 12 240 >> logs/log_12c_240v.txt 2>&1
perf stat -r 30 ./corrida 250 6 500 >> logs/log_06c_500v.txt 2>&1
