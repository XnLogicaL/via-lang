import utils
import sys
import subprocess

def main():
    if len(sys.argv) < 2:
        utils.log_message("usage: python3 script.py <path-to-executable>")
        return

    path=sys.argv[1]
    x=0

    while True:
        try:
            subprocess.run("./" + path,
                                    check=True,
                                    shell=True,
                                    stdout=subprocess.PIPE
                                    )
        except subprocess.CalledProcessError as e:
            pass

        print(f"fuzz attempt #{x} complete")
        x+=1

if __name__ == "__main__":
    main()
    