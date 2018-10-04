import matplotlib.pyplot as plt

############################
def getX(line) :
	start = line.find('x="')
	if start == -1 :
		return -999
	start += 3

	end = line.find('y="')
	if end == -1 :
		return -999
	end -= 2

	strX = line[start:end]
	return int(strX)
############################

############################
def getY(line) :
	start = line.find('y="')
	if start == -1 :
		return 
	start += 3

	end = line.find('/>')
	if end == -1 :
		return
	end -= 2

	strY = line[start:end]
	return int(strY)
############################

def draw_glif(name) :
	try :
		glif = open('cid3644.glif')
		lines = [line.rstrip('\n') for line in glif]

		listX = []
		listY = []
		fig = plt.figure()
		fig.suptitle(name)
		color = ['#f44242', '#f4a142', '#ebf442', '#74f442', '#424bf4', '#ad42f4']
		ind = 0
		for line in lines :
			if '</contour>' in line :
				for i in range(len(listX)) :
					if i == len(listX) - 1 :
						j = 0
					else :
						j = i + 1
					plt.plot([listX[i], listX[j]], [listY[i], listY[j]], color=color[ind])
					print('[' , listX[i] , ',' , listY[i] , '], [' , listX[j] , ',' , listY[j] , ']')

				listX.clear()
				listY.clear()
				print('-----------------------------')
				ind += 1
				continue

			if 'type' not in line :
				continue
		
			x = getX(line)
			y = getY(line)
			if x != -999 and y != -999 :
				listX.append(getX(line))
				listY.append(getY(line))

		plt.show()
		return

	except IOError :
		print('Could not read file :', name)
		return

glifName = input('glif number: ')
draw_glif('cid' + glifName + 'glif')







