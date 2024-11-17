from ansi_esc_seq import ANSI_escape_sequences as AES
from os import system

i = 0

def iota():
    global i
    i += 1
    return i

class test_case:
    cmd: str
    file: str
    status: bool
    uid: int

    def __init__(self, file_path, cmd):
        self.cmd = cmd
        self.file = file_path
        self.uid = iota()
    
    def run(self):
        print("[TEST CASE #%s]: %sTesting...%s" % self.uid, AES.WARNING, AES.ENDC)

        result = system(self.cmd + self.file)
        result >>= 8  # The return code is specified in the second byte

        if result != 0:
            self.status = False    
        
        print("[TEST CASE #%s]: %s%s%s" % 
            (AES.OKGREEN if self.status else AES.FAIL),
            ("Passed" if self.status else "Failed"),
            AES.ENDC
        )