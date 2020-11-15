import sys
import re

string = sys.argv[1]
pattern = re.compile("(^[^\.]+$|^Nominal.*$)")
result = pattern.match(string)
print result.groups()
