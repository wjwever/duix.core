import os

def read_header(fin):
	headers = []
	with open(fin) as f:
		for line in f:
			line = line.strip()
			if line.startswith('#include') and '.h' in line:
				if '<' in line:
					arr = line.split('<')
					for ele in arr:
						if '.h' in ele:
							ele = ele.split('>')[0].split('/')[-1]
							#print(fin,":", ele)	
							headers.append(ele)
				else:
					arr = line.split('"')	
					for ele in arr:
						if '.h' in ele:
							ele = ele.split('/')[-1]
							#print(fin,":", ele)
							headers.append(ele)						
	return headers


all_headers = {}
walk_tree = os.walk(".")
for dir, sub_dir, files in walk_tree:
	for file in files:
		if file.endswith('.h'):
			read_header(os.path.join(dir, file))
			if file in all_headers and all_headers[file] != os.path.join(dir, file):
				print('duplicate', all_headers[file], os.path.join(dir, file))
			all_headers[file] = os.path.join(dir, file)

all_headers['main.cpp'] = 'main.cpp'
all_headers['edge_render.cpp'] = 'edge_render.cpp'
book = {}
def run(fin, tab):	
	if fin in book:
		return
	newf = all_headers.get(fin, fin)
	print(str(tab) + '  ' * tab + newf)
	book[fin] = 1
	if fin in all_headers:
		fin = all_headers[fin]
		inc = read_header(fin)
		for f in inc:
			run(f, tab+1)
		
	

run('edge_render.cpp', 0)
