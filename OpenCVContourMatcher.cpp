#include "ContourMatcher.h"

void OpenCVContourMatcher::loadImages(Image i1, Image i2){
	loadImages(i1.getData(),i1.getHeight(),i1.getWidth(),i2.getData(),i2.getHeight(),i2.getWidth());
	return;
}

void OpenCVContourMatcher::loadImages(unsigned char* i1, unsigned int height1, unsigned int width1, unsigned char* i2, unsigned int height2, unsigned int width2){
	/*	
		!!! This method of constructing cv::Mat objects requires that the 
		image arrays pointed to are managed seperately!!!
	*/
		try{
			if (height1 != height2 | width1 != width2){
				throw 3;
			}
			image1 = cv::Mat(matrixRowNum,matrixColNum,cv::CV_8UC(3),i1);
			image2 = cv::Mat(matrixRowNum,matrixColNum,cv::CV_8UC(3),i2);
		} catch (int e) {
			ERR << "In OpenCVContourMatcher::loadImages: Cannot compare images of different sizes. Error code " << e << "\n";
		}
	return;
}

Plane OpenCVContourMatcher::convertDMatchesToPlane(std::vector<cv::Keypoint> features, std::vector<cv:DMatch> matches,int imageId = 0){
	Plane plane;
	//	std::vector<PixelLoc> planePoints;
	// convert DMatches to tbd set of points
	// Gets the 
	for (int i = 0; i < matches.size; i++){
		// queryIdx is in the "left" image 
		PixelLoc pt1(features[matches[i].queryIdx].pt.x,features[matches[i].queryIdx].pt.y);
		plane.leftImage.push_back(pt1);
		// trainIdx is in the "right" image
		PixelLoc pt2(features[matches[i].trainIdx].pt.x,features[matches[i].trainIdx].pt.y);
		plane.rightImage.push_back(pt2);
	}
	return planePoints;
}


std::vector<cv::Keypoint> OpenCVContourMatcher::detectFeaturePoints(cv::Mat contour){
	cv::SiftFeatureDetector detector;
	std::vector<cv::Keypoint> contourFeatures;
	detector.detect(contour,contourFeatures);
	return contourFeatures;
}


cv::Mat OpenCVContourMatcher::extractDescriptors(cv::Mat contour, std::vector<cv::Keypoint> contourFeatures){
	cv::SiftDescriptorExtractor extractor;
	cv::Mat contourDescriptors;
	extractor.compute(contour,contourFeatures,contourDescriptors);
	return contourDescriptors;
}

std::vector<PixelLoc> OpenCVContourMatcher::compare(Contour contour1, Contour contour2, MATCHER_TYPE m = FLANN){
	try {
		if ( m == FLANN) {
			compareFLANN(contour1, contour2);
		} else if ( m == BF ){
			compareBruteForce(contour1, contour2);
		} else {
			throw 2;
		}
	} catch (int e) {
		ERR << "In OpenCVContourMatcher::compare: Matcher not defined for OpenCVContourMatcher. Exitcode " << e << "\n";
	}
}

Plane OpenCVContourMatcher::compareFLANN(Contour contour1, Contour contour2){
	//Convert from contour class to matrix
	if (determineLargerContour(contour1,contour2) == 1 ){
		cv::Mat c1 = convertToMatrix(contour1);
		cv::Mat c2 = convertToMatrix(contour2);
	} else {
		cv::Mat c1 = convertToMatrix(contour1);
		cv::Mat c2 = convertToMatrix(contour2);
	}
	// Detect feature points of contour 1 and contour 2
	std::vector<cv::Keypoint> feat1 = detectFeaturePoints(c1);
	std::vector<cv::Keypoint> feat2 = detectFeaturePoints(c2);
	// Extract feature descriptors
	cv::Mat desc1 = extractDescriptors(c1,feat1);
	cv::Mat desc2 = extractDescriptors(c2,feat2);
	// Match feature descriptors and refine matches then return as vector of PixelLocations
	std::vector<cv::DMatch> matches = matchFLANN(desc1,desc2);
	Plane planePoints = convertDMatchesToSetOfPoints(refineMatches(matches,THRESHOLD));
	return planePoints;
}

Plane OpenCVContourMatcher::compareBruteForce(Contour contour1, Contour contour2){
	//Convert from contour class to matrix
	cv::Mat c1 = convertToMatrix(contour1);
	cv::Mat c2 = convertToMatrix(contour2);
	// Detect feature points of contour 1 and contour 2
	std::vector<cv::Keypoint> feat1 = detectFeaturePoints(c1);
	std::vector<cv::Keypoint> feat2 = detectFeaturePoints(c2);
	// Extract feature descriptors
	cv::Mat desc1 = extractDescriptors(c1,feat1);
	cv::Mat desc2 = extractDescriptors(c2,feat2);
	// Match feature descriptors and refine matches then return as vector of PixelLocations
	std::vector<cv::DMatch> matches = matchBruteForce(desc1,desc2);
	Plane planePoints = convertDMatchesToSetOfPoints(refineMatches(matches,THRESHOLD));
	return planePoints;
}

void OpenCVContourMatcher::computeMaxAndMinDistances(std::vector<cv::DMatch> matches){
	for (int i = 0; i < matches.size(); i++){
		double dist = matches[i].distance;
		if ( dist < minDistance ) minDistance = dist;
		if ( dist > maxDistance ) maxDistance = dist;
	}
	return;
}

std::vector<cv::DMatch> OpenCVContourMatcher::refineMatches(std::vector<DMatch> matches, int threshold){
	std::vector<cv::DMatch> bestMatches;
	// Determine how close to their expected locations the features 
	computeMaxAndMinDistances(matches);
	for (int i = 0; i < matches.size(); i++ ){
		if (matches[i].distance < minDistance * threshold){
			bestMatches.push_back(matches[i]);
		}
	}
	return bestMatches;
}

// Takes two descriptor matrices
std::vector<cv::DMatch> OpenCVContourMatcher::matchBruteForce(cv::Mat desc1, cv::Mat desc2){
	BruteForceMatcher matcher;
	std::vector<cv::DMatch> matches;
	matcher.match(desc1,desc2,matches);
	return matches;
}

// Takes two descriptor matrices
std::vector<cv::DMatch> OpenCVContourMatcher::matchFLANN(cv::Mat desc1, cv::Mat desc2){
	FlannBasedMatcher matcher;
	std::vector<cv::DMatch> matches;
	matcher.match(desc1,desc2,matches)
	return matches;
}