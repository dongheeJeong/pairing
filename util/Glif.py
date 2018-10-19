import os
import sys
import copy
from xml.dom import minidom

class Contour:
	def __init__(self, contour, pointList):
		self.pointList = pointList
		self.numLinePoints = 0
		self.numCurvPoints = 0
		self.contour = contour
		self.countPoints()

	def countPoints(self):
		for point in self.pointList:
			if point.hasAttribute('type') :
				if point.attributes['type'].value[:] == 'line':
					self.numLinePoints += 1
				elif point.attributes['type'].value[:]  ==  'curve':
					self.numCurvPoints += 1

	def getTargetPoint(self):
		tarPointList = []
		for ind, point in enumerate(self.pointList):
			if point.hasAttribute('smooth'):
				p = self.pointList[ind+1]
				if p.hasAttribute('type') and p.attributes['type'].value[:] == 'line':
					tarPointList.append(p)

				p = self.pointList[ind-1]
				if p.hasAttribute('type') and p.attributes['type'].value[:] == 'line':
					tarPointList.append(p)

		for p1 in tarPointList:
			for p2 in tarPointList:
				if p1 == p2:
					continue
				if p1.attributes['x'].value == p2.attributes['x'].value:
					tarPointList.clear()
					tarPointList.append(p1)
					tarPointList.append(p2)
					break
			else:
				continue
			break
		return tarPointList

	def getInnerPointList(self, tarPointList):
		InnerPointList = []
		indexList = []
		indexList.append(self.pointList.index(tarPointList[0]))
		indexList.append(self.pointList.index(tarPointList[1]))
		
		for index in indexList:
			p = self.pointList[index+1]
			if not p.hasAttribute('smooth'):
				InnerPointList.append(p)
				InnerPointList.append(self.pointList[index+2])
			else:
				InnerPointList.append(self.pointList[index-1])
				InnerPointList.append(self.pointList[index-2])

		return InnerPointList
	
	def print(self):
		print('numLinePoints: ', self.numLinePoints, 'numCurvPoints: ' , self.numCurvPoints)

class Glif:
	def __init__(self, dirPath, fileName):
		self.contourList = []
		self.fileName = fileName
		self.parseGlif(dirPath, fileName)

	def parseGlif(self, dirPath, fileName):
		xmldoc = minidom.parse(dirPath + fileName)
		self.xmldoc = xmldoc
		self.doParse(xmldoc)

	def doParse(self, xmldoc):
		contours = xmldoc.getElementsByTagName('contour')
		for contour in contours:
			pointList = contour.getElementsByTagName('point')
			c = Contour(contour, pointList)
			self.contourList.append(c)

	def getTargetContour(self):
		for contour in self.contourList:
			if contour.numLinePoints == 14 and contour.numCurvPoints == 2:
				return contour

	def print(self):
		for ind, contour in enumerate(self.contourList):
			print('contour[', ind, ']')
			contour.print()
			for point in contour.pointList:
				print(point.toxml('utf-8'))
			print(' ')
		
def changeCoordinate(pointList):
	distance = abs(int(pointList[0].attributes['y'].value) - int(pointList[1].attributes['y'].value))
	for point in pointList:
		x = int(point.getAttribute('x'))
		point.setAttribute('x', str(x + distance//2))

# have to detect line : 14, curve : 2
glifList = []
dirPath = '/Users/stevejobs/Desktop/python3/'

for fileName in os.listdir(dirPath):
	if fileName.endswith('.glif') and fileName.startswith('cid'):
		g = Glif(dirPath, fileName)
		glifList.append(g)

for g in glifList:
	print(g.fileName)
	tarContour = g.getTargetContour()
	if tarContour is None:
		print(g.fileName + ' has no target contour')
		continue
	tarPointList = tarContour.getTargetPoint()
	innerPointList = tarContour.getInnerPointList(tarPointList)
	outerPointList = [point for point in tarContour.pointList if point not in innerPointList]

	outline = g.xmldoc.getElementsByTagName('outline')[0]
	
	innerContour = copy.deepcopy(tarContour.contour)
	outerContour = copy.deepcopy(tarContour.contour)

	for point1 in outerPointList:
		for point2 in innerContour.getElementsByTagName('point'):
			if point1.toxml() == point2.toxml():
				innerContour.removeChild(point2)

	for point1 in innerContour.getElementsByTagName('point'):
		for point2 in outerContour.getElementsByTagName('point'):
			if point1.toxml() == point2.toxml():
				outerContour.removeChild(point2)

	ind = []
	for index, point in enumerate(outerContour.getElementsByTagName('point')):
		for point2 in tarPointList:
			if point.toxml() == point2.toxml():
				ind.append(index)
		
	changeCoordinate([outerContour.getElementsByTagName('point')[ind[0]], outerContour.getElementsByTagName('point')[ind[1]]])

	outline.removeChild(tarContour.contour)
	outline.appendChild(innerContour)
	outline.appendChild(outerContour)

	f = open('test'+g.fileName, 'w')
	xmlcontent = g.xmldoc.toprettyxml()
	xmlcontentList = xmlcontent.split('\n')

	for xmlcontent in xmlcontentList:
		if xmlcontent.isspace() == True:
			continue
		else:
			f.write(xmlcontent + '\n')
	f.close()


