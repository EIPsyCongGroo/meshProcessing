#include "remesh.h"

double targetL(MyMesh& mesh)
{
    Eigen::MatrixXd V;
    Eigen::MatrixXi F;
    openMesh_to_igl(mesh, V, F);
    Eigen::Vector3d m = V.colwise().minCoeff();
    Eigen::Vector3d M = V.colwise().maxCoeff();
    double L = (M-m).norm();
    return L;
}

void EdgeSplitFunc(MyMesh & mesh, double targetLength)
{
    double max = targetLength * 4 / 3;
    MyMesh::EdgeIter End(mesh.edges_end());
    OpenMesh::Vec3d midPt;
    for (MyMesh::EdgeIter iter = mesh.edges_sbegin(); iter != End; ++iter) {
        if (max < (mesh.point((*iter).v0()) - mesh.point((*iter).v1())).length())
        {
            midPt = (mesh.point((*iter).v0()) + mesh.point((*iter).v1())) / 2;
            mesh.split(*iter, (mesh.point((*iter).v0()) + mesh.point((*iter).v1())) / 2);
        }
    }
    mesh.garbage_collection();
}

void EdgeColapaseFunc(MyMesh& mesh, double targetLength)
{
    double min = targetLength * 4 / 5;
    MyMesh::EdgeIter End(mesh.edges_end());
    for (auto iter = mesh.edges_sbegin(); iter != End; ++iter)
    {
        if ((*iter).is_boundary() || (*iter).v0().is_boundary() || (*iter).v1().is_boundary() )
            continue;
        if (min > (mesh.point((*iter).v0()) - mesh.point((*iter).v1())).length())
        {
            if (mesh.is_collapse_ok((*iter).h0()))
                mesh.collapse((*iter).h0());
            if (mesh.is_collapse_ok((*iter).h1()))
                mesh.collapse((*iter).h1());
        }
    }
    mesh.garbage_collection();
}

void EdgeFlipFunc(MyMesh& mesh)
{
    double origin, after;
    MyMesh::EdgeIter iter;
    for (iter = mesh.edges_begin(); iter != mesh.edges_end(); ++iter) 
    {
        // 获取当前边缘的两个相邻面
        MyMesh::FaceHandle fh1 = mesh.face_handle(mesh.halfedge_handle(*iter, 0));
        MyMesh::FaceHandle fh2 = mesh.face_handle(mesh.halfedge_handle(*iter, 1));

        // 计算两个相邻面的法向量
        OpenMesh::Vec3f n1 = mesh.normal(fh1);
        OpenMesh::Vec3f n2 = mesh.normal(fh2);

        // 计算两个法向量的夹角
        float angle = acos(n1 | n2);

        // 判断是否为锐利边缘
        if (angle > 0.5)
        {
            if (mesh.is_flip_ok(*iter))
            {
                origin = (mesh.point((*iter).v0()) - mesh.point((*iter).v1())).length();
                mesh.flip(*iter);
                after = (mesh.point((*iter).v0()) - mesh.point((*iter).v1())).length();
                if (origin < after)
                    mesh.flip(*iter);
            }
        }
        
    }
    mesh.garbage_collection();
}

void MeshSmoothFunc(MyMesh& mesh)
{
    OpenMesh::Smoother::SmootherT< MyMesh >::Component com =
        OpenMesh::Smoother::SmootherT< MyMesh >::Tangential;
    OpenMesh::Smoother::SmootherT< MyMesh >::Continuity con =
        OpenMesh::Smoother::SmootherT< MyMesh >::C0;

    OpenMesh::Smoother::JacobiLaplaceSmootherT<MyMesh> smoother(mesh);
    smoother.initialize(com, con);
    smoother.smooth(5);
}

void Remesh(MyMesh& mesh, double beta)
{
    mesh.request_vertex_status();
    mesh.request_edge_status();
    mesh.request_halfedge_status();
    mesh.request_face_status();
    EdgeFlipFunc(mesh);
    double targetLength = beta*targetL(mesh);
    for (int i = 0; i < 3; i++) 
    {
        EdgeSplitFunc(mesh, targetLength);
        EdgeColapaseFunc(mesh, targetLength);
        EdgeFlipFunc(mesh);
        MeshSmoothFunc(mesh);
        mesh.garbage_collection();
    }
}
