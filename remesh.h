#pragma once
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Tools/Smoother/JacobiLaplaceSmootherT.hh>
#include "Base.h"

typedef OpenMesh::TriMesh_ArrayKernelT<> MyMesh;

double targetL(MyMesh& mesh);
void Remesh(MyMesh& mesh, double beta);
void EdgeSplitFunc(MyMesh& mesh, double targetLength);
void EdgeColapaseFunc(MyMesh& mesh, double targetLength);
void EdgeFlipFunc(MyMesh& mesh);
void MeshSmoothFunc(MyMesh& mesh);

