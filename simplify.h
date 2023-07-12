#pragma once
#include "Base.h"
#include <OpenMesh/Core/Mesh/Traits.hh>
#include <OpenMesh/Core/Utils/PropertyManager.hh>

struct edge_Collapse_structure
{
    MyMesh::HalfedgeHandle hf;
    MyMesh::Point np;
    MyMesh::VertexHandle vto;
    MyMesh::VertexHandle vfrom;
    //下面两个用来判断该点对是否已被更新过的点对取代
    int vto_flag = 0;
    int vfrom_flag = 0;
    Eigen::Matrix4d Q_new;
    float k;
    float cost;
    bool operator<(const edge_Collapse_structure& a) const
    {
        return  a.cost < cost;
    }
    bool isDegenerate(MyMesh::HalfedgeHandle he, MyMesh& mesh)
    {
        MyMesh::Point p1, p2, p3;
        p1 = mesh.point(mesh.from_vertex_handle(he));
        p2 = mesh.point(mesh.to_vertex_handle(he));
        p3 = mesh.point(mesh.to_vertex_handle(mesh.next_halfedge_handle(he)));
        return p1 == p2 || p2 == p3 || p1 == p3;
    }
    bool isInedge(Eigen::Vector4f v0, Eigen::Vector4f v1, Eigen::Vector4f v2)
    {
        MyMesh::Point p;
        for (int i = 0; i < 3; i++)
        {
            p[i] = (v0[i] - v2[i]) / (v1[i] - v2[i]);
            if (p[i]>0 && p[i]<1)
            {
                return true;
            }
        }
    }
};

// t is a threshold parameter
void Surface_Simplification(MyMesh& mesh, float ratio);



