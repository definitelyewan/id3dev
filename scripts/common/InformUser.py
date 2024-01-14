
class Inform:

    def __init__(self, step):
        self.step = step
    
    def message(self, msg):
        print("[" + str(self.step) + "] " + msg)
        self.step += 1
    
            