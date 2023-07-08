#include "simplify.h"
#include "Base.h"
#include <queue>
#include<iostream>
Eigen::MatrixXd Surface_Simplification(MyMesh& mesh, float ratio, Eigen::MatrixXd uv)
{
    assert(ratio >= 0 && ratio <= 1);
    int it_num = (1.0f - ratio) * mesh.n_vertices();
    
 
    //1. Compute the Q matrices for all the initial vertices
    auto Qv = OpenMesh::makeTemporaryProperty<OpenMesh::VertexHandle, Eigen::Matrix<double, 6, 6>>(mesh);
    auto v = OpenMesh::makeTemporaryProperty<OpenMesh::VertexHandle, Eigen::Vector<double, 6>>(mesh);
    auto flag = OpenMesh::makeTemporaryProperty<OpenMesh::VertexHandle, int>(mesh); 
    auto A = OpenMesh::makeTemporaryProperty<OpenMesh::FaceHandle, Eigen::Matrix<double, 5, 5>>(mesh);
    auto b = OpenMesh::makeTemporaryProperty<OpenMesh::FaceHandle, Eigen::Vector<double, 5>>(mesh);
    auto c = OpenMesh::makeTemporaryProperty<OpenMesh::FaceHandle, double>(mesh);
    auto Q = OpenMesh::makeTemporaryProperty<OpenMesh::FaceHandle, Eigen::Matrix<double, 6, 6>>(mesh);
    auto vt = OpenMesh::makeTemporaryProperty<OpenMesh::VertexHandle, Eigen::Vector2d>(mesh);

    for (MyMesh::VertexIter vit = mesh.vertices_begin(); vit != mesh.vertices_end(); ++vit)
    {
        MyMesh::VertexHandle vh = vit;
        vt[*vit] = uv.row(vh.idx());
        
        v[*vit][0] = mesh.point(*vit)[0];
        v[*vit][1] = mesh.point(*vit)[1];
        v[*vit][2] = mesh.point(*vit)[2];
        v[*vit][3] = uv.row(vh.idx())[0];
        v[*vit][4] = uv.row(vh.idx())[1];
        v[*vit][5] = 1.0;

        //std::cout << v[*vit] << "\n";
    }

    for (MyMesh::FaceIter fit = mesh.faces_begin(); fit != mesh.faces_end(); ++fit)
    {
        Eigen::VectorXd p = v[fit->halfedge().from()].head(5);
        Eigen::VectorXd e1 = (v[fit->halfedge().to()].head(5) - p).normalized();
        Eigen::VectorXd e2 = (v[fit->halfedge().next().to()].head(5) - p);
        e2 = (e2 - (e1.dot(e1) * e1)).normalized();
        A[*fit] = Eigen::MatrixXd::Identity(5, 5) - e1 * e1.transpose() - e2 * e2.transpose();
        b[*fit] = (p.dot(e1)) * e1 + (p.dot(e2)) * e2 - p;
        c[*fit] = p.dot(p) - (p.dot(e1)) * (p.dot(e1)) - (p.dot(e2)) * (p.dot(e2));
        Q[*fit].block(0, 0, 5, 5) = A[*fit];
        Q[*fit].block(0, 5, 5, 1) = b[*fit];
        Q[*fit].block(5, 0, 1, 5) = b[*fit].transpose();
        Q[*fit](5, 5) = c;
        //std::cout << Q[*fit] << "\n";
    }
    for (MyMesh::VertexIter vit = mesh.vertices_begin(); vit != mesh.vertices_end(); ++vit)
    {
        Eigen::Matrix<double, 6, 6> mat;
        mat.setZero();
        for (MyMesh::VertexFaceIter vf_it = mesh.vf_iter(*vit); vf_it.is_valid(); ++vf_it)
        {
            mat += Q[*vf_it];
        }
        Qv[*vit] = mat;  
        flag[*vit] = 0;
        //std::cout << Qv[*vit] << "\n";
    }
    //std::cout << "caculate curvature done" << std::endl;
    // 2. Select all valid pairs (only vertices in an edge are considered)
    // 3. Compute the optimal contraction target
    std::priority_queue <edge_Collapse_structure, std::vector<edge_Collapse_structure>, std::less<edge_Collapse_structure>> q;
    for (MyMesh::EdgeIter eit = mesh.edges_begin(); eit != mesh.edges_end(); ++eit)
    {
        Eigen::Matrix<double, 6, 6> tQ = Qv[eit->v0()] + Qv[eit->v1()];
        Eigen::Vector<double, 6> b(0.0, 0.0, 0.0, 0.0, 0.0, 1.0);

        //std::cout << tQ << std::endl;

        Eigen::FullPivLU<Eigen::Matrix<double, 6, 6>> lu(tQ);
        Eigen::Vector<double, 6> vnew;
        // if is invertible, solve the linear equation
        if (lu.isInvertible())
        {

            vnew = tQ.inverse() * b;
        }
        // else take the midpoint
        else
        {
            vnew = (v[eit->v0()] + v[eit->v1()]) / 2.0;
        }
        //std::cout << vnew << std::endl;
        edge_Collapse_structure ts;
        ts.hf = eit->halfedge(0);
        ts.cost = abs(vnew.transpose() * tQ * vnew);
        if (ts.cost < 0)
        {
            std::cout << "Minus cost:" << ts.cost << std::endl;
            //std::cout << "vnew:" << vnew << std::endl;
            //std::cout << "newQ:" << newQ << std::endl;
        }

        //std::cout << "first cost:" << ts.cost << std::endl;
        ts.np = vnew.head(5);
        ts.vto = eit->halfedge(0).to();
        ts.vfrom = eit->halfedge(0).from();
        ts.Q_new = tQ;
        q.push(ts);
        
        
    }

    mesh.request_vertex_status();
    mesh.request_edge_status();
    mesh.request_face_status();
 
    std::cout << "simplify begin..." << std::endl;
    int i = 0;
    while (i < it_num)
    {
        edge_Collapse_structure s = q.top();
        q.pop();
        if (mesh.status(s.vfrom).deleted() || mesh.status(s.vto).deleted())
            continue;
        if (s.vto_flag != flag[s.vto] || s.vfrom_flag != flag[s.vfrom])
            continue;
        MyMesh::VertexHandle tvh;
        MyMesh::HalfedgeHandle ophf = mesh.opposite_halfedge_handle(s.hf);
        if (mesh.is_collapse_ok(s.hf) && !mesh.is_boundary(s.hf))
        {
                    mesh.collapse(s.hf);
                    tvh = s.vto;
                    flag[s.vto] ++;
                    flag[s.vfrom] ++;
     
        }

        
        else if (mesh.is_collapse_ok(ophf) && !mesh.is_boundary(ophf))
        {
                    mesh.collapse(ophf);
                    tvh = s.vto;
                    flag[s.vto] ++;
                    flag[s.vfrom] ++;
        }
        else
        {
            continue;
        }

        MyMesh::Point p(s.np[0], s.np[1], s.np[2]);
        mesh.set_point(tvh, p);
        Qv[tvh] = s.Q_new;
        v[tvh].head(5) = s.np;
        v[tvh][6] = 1.0;
        vt[tvh] = s.np.segment(3,2);
        for (MyMesh::VertexOHalfedgeIter vh_it = mesh.voh_iter(tvh); vh_it.is_valid(); ++vh_it)
        {
            MyMesh::VertexHandle tt = vh_it->to();
            Eigen::Matrix<double, 6, 6> tQ = s.Q_new + Qv[tt];
            Eigen::Vector<double, 6> b(0.0, 0.0, 0.0, 0.0, 0.0, 1.0);
            Eigen::FullPivLU<Eigen::Matrix<double, 6, 6>> lu(tQ);
            Eigen::VectorXd vnew;
            // if is invertible, solve the linear equation
            if (lu.isInvertible())
            {
                vnew = tQ.inverse() * b;
            }
            // else take the midpoint
            else
            {
                vnew = (v[tvh] + v[tt]) / 2.0;
            }
            //std::cout << vnew << std::endl;
            edge_Collapse_structure ts;
            ts.hf = *vh_it;
            //std::cout << "update curvature:" << ts.k << std::endl;
            ts.cost = abs(vnew.transpose() * tQ * vnew);
            //std::cout << "update cost:" << ts.cost << std::endl;
            ts.np = vnew.head(5);
            ts.vto = tt;           
            ts.vto_flag = flag[tt];
            ts.vfrom = tvh;
            ts.vfrom_flag = flag[tvh];
            ts.Q_new = tQ;
            q.push(ts);
            
        }
        i++;
        
    }
    mesh.garbage_collection();
    Eigen::MatrixXd newuv(mesh.n_vertices(), 2);
    for (MyMesh::VertexIter vit = mesh.vertices_sbegin(); vit != mesh.vertices_end(); ++vit)
    {
        MyMesh::VertexHandle vh = vit;
        newuv.row(vh.idx()) = vt[*vit];
    }
    //std::cout << newuv << std::endl;
    return newuv;
}
