# Articulated Pose Estimator

Implementation of the papers:
  
**Motion Capture of Hands in Action using Discriminative Salient Points** <BR>
L. Ballan, A. Taneja, J. Gall, L. Van Gool, M. Pollefeys<BR>
ECCV 2012, Firenze, Italy<BR>
http://lucaballan.altervista.org/pdfs/ballanECCV12.pdf<BR>

and

**Marker-less Motion Capture of Skinned Models in a Four Camera Set-up using Optical Flow and Silhouettes**<BR>
L. Ballan and G. M. Cortelazzo<BR>
3DPVT 2008, Atlanta, GA, USA<BR>
http://lucaballan.altervista.org/pdfs/3DPVT08.pdf<BR>

<BR>
If you are using this code, please cite the above papers.
<BR>
<BR>

![Alt text](sample1.png?raw=true "Title")
![Alt text](sample2.jpg?raw=true "Title")
![Alt text](sample3.png?raw=true "Title")


INSTALL
----
 - Install Opencv in "Common Libs\opencv_win"
 - Install newmat in "Common Libs\newmat"
 - Install gsl in "Common Libs\gsl"
 - Install glew in "Common Libs\glew"
 - Install CLAPACK in "Common Libs\CLAPACK"
 - Install Levenberg-Marquardt in "Common Libs\Levenberg-Marquardt" (LEVMAR version 2.5 By Manolis Lourakis)
 - Install lp_solve in "Common Libs\lp_solve" (http://lpsolve.sourceforge.net/)
 - Install all the related dlls in bin32 and bin64
 - Compile src/prj/gammalib/gammalib.vcxproj
 - Compile src/prj/gammalib/ArticulatedPoseEstimator/ArticulatedPoseEstimator.vcxproj
 - Compile src/prj/gammalib/ArticulatedPoseEstimator/Viewer.vcxproj
