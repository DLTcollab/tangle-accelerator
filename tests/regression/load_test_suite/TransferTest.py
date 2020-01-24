from common import *
from multiprocessing import Process, Manager
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plot
import numpy as np
import json
import time


class SendTransfer():
    def __init__(self, transfer_num, client_min, client_max, stepsize):
        self.time_cost = []
        self.error_rate = []
        self.transfer_num = transfer_num
        self.client_min = client_min
        self.client_max = client_max
        self.stepsize = stepsize

    def _send_transfer(self, sample_time_cost, sample_error_rate,
                       transfer_num):
        process_time_cost = []
        process_error_count = 0.0
        rand_msg = gen_rand_trytes(30)
        rand_tag = gen_rand_trytes(27)
        rand_addr = gen_rand_trytes(81)
        for i in range(transfer_num):
            start_time = time.time()
            post_data = {
                "value": 0,
                "message": rand_msg,
                "tag": rand_tag,
                "address": rand_addr
            }
            post_data_json = json.dumps(post_data)
            ret = API("/transaction", post_data=post_data_json)

            if ret["status_code"] != 200:
                process_error_count += 1
            process_time_cost.append(time.time() - start_time)
        sample_error_rate += [process_error_count / transfer_num]
        sample_time_cost += process_time_cost

    def test(self):
        manager = Manager()
        for client_num in range(self.client_min, self.client_max + 1,
                                self.stepsize):
            sample_time_cost = manager.list()
            sample_error_rate = manager.list()
            process_list = []

            for i in range(client_num):
                process_list.append(
                    Process(
                        target=self._send_transfer,
                        args=(sample_time_cost, sample_error_rate,
                              self.transfer_num)))
                process_list[i].start()

            for i in range(client_num):
                process_list[i].join()

            self.time_cost.append(sample_time_cost)
            self.error_rate.append(sample_error_rate)

    def dumpfile(self, file_name):
        write_file = open(file_name, 'w')
        for i, client_num in enumerate(
                range(self.client_min, self.client_max + 1, self.stepsize)):
            write_file.write(
                'Client:%s\tTransfer:%s\tStandard Dev:%.3f\tAvg Time:%.3f\tError Rate:%.3f\n'
                % (client_num, self.transfer_num,
                   statistics.stdev(self.time_cost[i]),
                   statistics.mean(self.time_cost[i]),
                   statistics.mean(self.error_rate[i])))
        write_file.close()

    def plot(self, file_name):
        x = []
        y = []
        for i, client_num in enumerate(
                range(self.client_min, self.client_max + 1, self.stepsize)):
            x += [client_num] * len(self.time_cost[i])
            y += self.time_cost[i]
        plot.title("Transfer Request Response Time")
        plot.xlabel("Client Number")
        plot.ylabel("Response Time(s)")
        plot.xticks(
            np.arange(
                self.client_min, self.client_max + 1, step=self.stepsize))
        plot.scatter(x, y, s=[5] * len(x))
        plot.savefig(file_name)
