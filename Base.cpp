#include "Base.h"

MyMesh igl_to_openMesh(Eigen::MatrixXd& V, Eigen::MatrixXi& F)
{
    MyMesh mesh;
    for (int i = 0; i < V.rows(); ++i)
    {
        mesh.add_vertex(MyMesh::Point(V(i, 0), V(i, 1), V(i, 2)));
    }
    for (int i = 0; i < F.rows(); ++i)
    {

        MyMesh::VertexHandle vhandle[3];
        vhandle[0] = mesh.vertex_handle(F(i, 0));
        vhandle[1] = mesh.vertex_handle(F(i, 1));
        vhandle[2] = mesh.vertex_handle(F(i, 2));
        std::vector<MyMesh::VertexHandle> face_vhandles;
        face_vhandles.clear();
        face_vhandles.push_back(vhandle[0]);
        face_vhandles.push_back(vhandle[1]);
        face_vhandles.push_back(vhandle[2]);
        mesh.add_face(face_vhandles);
        
    }
    return mesh;

}
void openMesh_to_igl(MyMesh& mesh, Eigen::MatrixXd& V, Eigen::MatrixXi& F)
{
    //��ʼ������
    V.setZero(mesh.n_vertices(), 3);
    F.setZero(mesh.n_faces(), 3);
    int i = 0;
    for (MyMesh::VertexIter v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); ++v_it, ++i)
    {
        V(i, 0) = mesh.point(*v_it)[0];
        V(i, 1) = mesh.point(*v_it)[1];
        V(i, 2) = mesh.point(*v_it)[2];
    }
    i = 0;
    for (MyMesh::FaceIter f_it = mesh.faces_begin(); f_it != mesh.faces_end(); ++f_it, ++i)
    {
        int j = 0;
        for (MyMesh::FaceVertexCCWIter fv_it = mesh.fv_ccwiter(*f_it); fv_it.is_valid(); ++fv_it, ++j)
        {
            F(i, j) = (*fv_it).idx();
        }
    }
}
float calc_angle(MyMesh::HalfedgeHandle he, MyMesh& mesh)
{
    MyMesh::Point p1, p2, p3;
    p1 = mesh.point(mesh.from_vertex_handle(he));
    p2 = mesh.point(mesh.to_vertex_handle(he));
    p3 = mesh.point(mesh.to_vertex_handle(mesh.next_halfedge_handle(he)));
    MyMesh::Point v1 = p1 - p3;
    MyMesh::Point v2 = p2 - p3;
    float a = OpenMesh::norm(v1);
    float b = OpenMesh::norm(p1 - p2);
    float c = OpenMesh::norm(v2);
    //return acosf((v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]) / (p1.norm() * p2.norm()));
    return acosf((c * c + a * a - b * b) / (2 * c * a));
}
float get_Voronoi_cell_area(MyMesh::VertexHandle vh, MyMesh& mesh)
{
    float area = 0.0;
    for (MyMesh::VertexOHalfedgeIter vh_it = mesh.voh_iter(vh); vh_it.is_valid(); ++vh_it)
    {
        float w = 0.0f;
        if (vh_it->is_valid())
        {
            w += 1.0f / tan(calc_angle(*vh_it, mesh));
        }


        if (vh_it->opp().is_valid())
        {
            w += 1.0f / tan(calc_angle(vh_it->opp(), mesh));
        }
        if (w < 0)
            w = 0;
        if (w > 20)
            w = 20;
        area += w * OpenMesh::sqrnorm(mesh.point(vh) - mesh.point(vh_it->to()));
    }
    area /= 8.0f;
    return area;
}
float calc_Gaussian_Curvature(MyMesh::VertexHandle vh, MyMesh& mesh)
{
   float ret=0.0;
    
    float area = get_Voronoi_cell_area(vh, mesh);
    float sum_angle = 0.0;
    for (MyMesh::VertexOHalfedgeIter vh_it = mesh.voh_iter(vh); vh_it.is_valid(); ++vh_it)
    {
        sum_angle += calc_angle(vh_it->next(), mesh);
    }
    ret = abs((2 * M_PI - sum_angle) / area);

    return ret;
}
Eigen::VectorXd calc_Gaussian_Curvature(MyMesh& mesh)
{
    Eigen::VectorXd ret(mesh.n_vertices());
    for (MyMesh::VertexIter v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); ++v_it)
    {
        float area = get_Voronoi_cell_area(*v_it, mesh);
        float sum_angle = 0.0;
        for (MyMesh::VertexOHalfedgeIter vh_it = mesh.voh_iter(*v_it); vh_it.is_valid(); ++vh_it)
        {
            sum_angle += calc_angle(vh_it->next(), mesh);
        }
        ret[v_it->idx()] = ((2 * M_PI - sum_angle) / area);
    }
    return ret;
}

double calc_sigma(MyMesh& mesh)
{
    double avg_edge_length = 0.0;
    double sum_squared_diff = 0.0;
    int num_edges = 0;

    for (MyMesh::EdgeIter e_it = mesh.edges_begin(); e_it != mesh.edges_end(); ++e_it) {
        MyMesh::EdgeHandle eh = *e_it;
        MyMesh::HalfedgeHandle heh = mesh.halfedge_handle(eh, 0);
        MyMesh::Point p0 = mesh.point(mesh.from_vertex_handle(heh));
        MyMesh::Point p1 = mesh.point(mesh.to_vertex_handle(heh));
        double edge_length = (p1 - p0).length();
        avg_edge_length += edge_length;
        num_edges++;
    }

    avg_edge_length /= num_edges;

    for (MyMesh::EdgeIter e_it = mesh.edges_begin(); e_it != mesh.edges_end(); ++e_it) {
        MyMesh::EdgeHandle eh = *e_it;
        MyMesh::HalfedgeHandle heh = mesh.halfedge_handle(eh, 0);
        MyMesh::Point p0 = mesh.point(mesh.from_vertex_handle(heh));
        MyMesh::Point p1 = mesh.point(mesh.to_vertex_handle(heh));
        double edge_length = (p1 - p0).length();
        double diff = edge_length - avg_edge_length;
        sum_squared_diff += diff * diff;
    }

    return sum_squared_diff / num_edges;
}

void mesh_filter(MyMesh& mesh, int iterations)
{
    auto fn = OpenMesh::makeTemporaryProperty<OpenMesh::FaceHandle, MyMesh::Point>(mesh);
    while (iterations-- > 0)
    {
        //Face normal filtering
         for (MyMesh::FaceIter fit = mesh.faces_begin(); fit != mesh.faces_end(); ++fit)
         {
            
            double alfa = 0.0;
            double beta = 0.0;
            double thet = M_PI / 9;
            double area = mesh.calc_face_area(*fit);

            mesh.request_face_normals();
            mesh.update_face_normals();
            MyMesh::Point ni = mesh.normal(*fit);
            MyMesh::Point ci = mesh.calc_face_centroid(*fit);
            MyMesh::Point ni_new(0, 0, 0);
            for (MyMesh::FaceFaceIter f_fit = mesh.ff_iter(*fit); f_fit.is_valid(); ++f_fit)
            {
                MyMesh::Point nj = mesh.normal(*f_fit);
                if (ni.dot(nj)> cos(thet))
                {
                    MyMesh::Point cj = mesh.calc_face_centroid(*f_fit);
                    alfa = exp(-(ci - cj).dot(ci - cj) / (2 * calc_sigma(mesh)));
                    beta = exp(-pow(((1.0 - ni.dot(nj)) / (1.0 - cos(thet))), 2));
                
                    ni_new += area * alfa * beta * ni;
                }

                fn[*fit] = ni_new.normalized();
                //std::cout << fn[*fit] << "\n";
            }
         }

        //Vertex position update
         for (MyMesh::VertexIter vit = mesh.vertices_begin(); vit != mesh.vertices_end(); ++vit)
         {
             MyMesh::Point vi = mesh.point(*vit);
             MyMesh::Point delta_vi(0, 0, 0);
             for (MyMesh::VertexFaceIter vf_it = mesh.vf_iter(*vit); vf_it.is_valid(); ++vf_it)
             {
                 delta_vi += (mesh.calc_face_centroid(*vf_it) - vi).dot(fn[*vf_it]) * mesh.normal(*vf_it);
             }
             vi += delta_vi / mesh.valence(*vit);
             mesh.set_point(*vit, vi);
         }
    }

    
   
}
