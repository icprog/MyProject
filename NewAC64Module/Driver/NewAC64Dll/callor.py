import traceback
import subprocess

try:
	subprocess.Popen( ["python.exe", "patParser.py", "TS.txt", "TS"] )
	# subprocess.Popen( ["python.exe", "patParser.py", "..\Daniel\FB.txt", "FB"] )
	# subprocess.Popen( ["python.exe", "patParser.py", "D:/aaron_work/project_workspace/PEB64/trunk/DOC/DesignInput/legacy/Chiperase.pat", "LEGACY"] )
except:
	print traceback.format_exc()
raw_input("Press ENTER to close window...\n")





