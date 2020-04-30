import glob
import random


class Generator:
    def __init__(self, inputdir, shuffle, size):
        self.shuffle = shuffle
        self.records = []
        self.current = 0
        filenames = glob.glob(inputdir + "/*.bin")
        for name in filenames:
            with open(name, "rb") as f:
                data = f.read()
            n = len(data)
            assert n % size == 0
            for i in range(0, n, size):
                self.records.append(data[i:i+size])
        print(f"Loaded {len(self.records)} records shuffle:{shuffle}")


    def __iter__(self):
        if self.shuffle:
            random.shuffle(self.records)
        self.current = 0
        return self


    def __next__(self):
        if self.current >= len(self.records):
            raise StopIteration
        data = self.records[self.current]
        self.current += 1
        return data


