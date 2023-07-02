#pragma once
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <igl/opengl/glfw/Viewer.h>
#include <vector>
typedef OpenMesh::TriMesh_ArrayKernelT<> MyMesh;
MyMesh igl_to_openMesh(Eigen::MatrixXd& V, Eigen::MatrixXi& F);
void openMesh_to_igl(MyMesh& mesh, Eigen::MatrixXd& V, Eigen::MatrixXi& F);
float calc_angle(MyMesh::HalfedgeHandle he, MyMesh& mesh);
float get_Voronoi_cell_area(MyMesh::VertexHandle vh, MyMesh& mesh);
float calc_Gaussian_Curvature(MyMesh::VertexHandle vh, MyMesh& mesh);
Eigen::VectorXd calc_Gaussian_Curvature(MyMesh& mesh);

