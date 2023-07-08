#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Tools/Subdivider/Uniform/LoopT.hh>
#include <OpenMesh/Tools/Subdivider/Uniform/Sqrt3T.hh>

#include <igl/opengl/glfw/Viewer.h>
#include <igl/opengl/glfw/imgui/ImGuiPlugin.h>
#include <igl/opengl/glfw/imgui/ImGuiMenu.h>
//#include <igl/opengl/glfw/imgui/ImGuiHelpers.h>
#include <igl/png/readPNG.h>
#include <igl/png/texture_from_png.h>


#include<fstream>
#include "Base.h"
#include "simplify.h"
#include "remesh.h"


typedef OpenMesh::TriMesh_ArrayKernelT<> MyMesh;
const int cache_size = 12;
Eigen::MatrixXd V[cache_size], uv_coords[cache_size];
Eigen::MatrixXi F[cache_size];

bool flag[cache_size] = { false };
MyMesh mesh;
double TargetLength = 0.008;
float alfa = 0.1;

int ti = 0;
bool key_down(igl::opengl::glfw::Viewer& viewer, unsigned char key, int modifier)
{
    if (key == '1')
    {
        if (ti < cache_size)
        {
            if (!flag[ti])
            {
                Surface_Simplification(mesh, 0.5, uv_coords[0]);
                std::cout << "vertices : " << mesh.n_vertices() << std::endl;;
                std::cout << "faces : " << mesh.n_faces() << std::endl;;
                openMesh_to_igl(mesh, V[ti], F[ti]);
                flag[ti] = true;
            }
            viewer.data().clear();
            viewer.data().set_mesh(V[ti], F[ti]);
            viewer.core().align_camera_center(V[ti], F[ti]);
            ti++;
        }

    }
    else if (key == '2')
    {
        if (ti > 1)
        {
            ti--;
            viewer.data().clear();
            viewer.data().set_mesh(V[ti], F[ti]);
            viewer.core().align_camera_center(V[ti], F[ti]);
        }
    }
    return false;
}

int main(int argc, char* argv[])
{

    OpenMesh::VPropHandleT< double > test;

    OpenMesh::IO::Options opt = 0x0040;


    //std::string fname = "./Release/OBJ/Tile_+005_+008_L20_0uuuu00.obj";
    std::string fname = "E:\\Assignment3\\Code\\models\\spot\\spot_triangulated_good.obj";
    std::string file = argc > 1 ? argv[1] : fname;
    
    OpenMesh::IO::read_mesh(mesh, file, opt);

    
    mesh.add_property(test);
    mesh.request_vertex_normals();
    mesh.request_face_normals();
    mesh.request_vertex_texcoords2D();
    if (mesh.has_vertex_texcoords2D())
    {
        std::cerr << "Has_vertex_texcoords" << "\n";
        uv_coords[0] = get_texture(file);
    }

    size_t lastdot = file.rfind('.');
    std::string texture_file = file.substr(0, (lastdot)) + "_1.jpg";
    std::ifstream mtlfile(texture_file);
    if (!mtlfile.is_open())
    {
        std::cerr << "Error: Cannot open file " << texture_file << std::endl;
    }
    

    /*mesh.update_face_normals();
    mesh.update_vertex_normals();*/
   
    igl::opengl::glfw::Viewer viewer;
    viewer.callback_key_down = &key_down;
    openMesh_to_igl(mesh, V[0], F[0]);

    std::cout << "vertices : " << mesh.n_vertices() << std::endl;
    std::cout << "faces : " << mesh.n_faces() << std::endl;
    // Attach a menu plugin
    igl::opengl::glfw::imgui::ImGuiPlugin plugin;
    viewer.plugins.push_back(&plugin);
    igl::opengl::glfw::imgui::ImGuiMenu menu;
    plugin.widgets.push_back(&menu);


    // Add content to the default menu window
    menu.callback_draw_viewer_menu = [&]()
    {
        // Draw parent menu content
        menu.draw_viewer_menu();
        if (ImGui::Button("Save Mesh", ImVec2(-1, 0)))
        {
            std::string fname = igl::file_dialog_save();

            if (fname.length() == 0)
                return;
            fname = fname + ".obj";

         
            OpenMesh::IO::write_mesh(mesh, fname);
        }

        if (ImGui::Button("Discard", ImVec2(-1, 0)))
        {
            if (ti >= 1)
            {
                ti--;
                viewer.data().clear();
                viewer.data().set_mesh(V[ti], F[ti]);
                viewer.core().align_camera_center(V[ti], F[ti]);
            }
        }


        // Add new group
        if (ImGui::CollapsingHeader("Mesh Prosessing", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Button("Sqrt3", ImVec2(-1, 0)))
            {
                
                OpenMesh::Subdivider::Uniform::Sqrt3T<MyMesh> Sqrt3;

                if (ti < cache_size)
                {
                    if (!flag[ti])
                    {
                        // Execute  subdivision steps
                        Sqrt3.attach(mesh);
                        Sqrt3(1);
                        Sqrt3.detach();
                        std::cout << "vertices : " << mesh.n_vertices() << std::endl;;
                        std::cout << "faces : " << mesh.n_faces() << std::endl;;
                        openMesh_to_igl(mesh, V[ti], F[ti]);
                        flag[ti] = true;
                    }
                    viewer.data().clear();
                    viewer.data().set_mesh(V[ti], F[ti]);
                    viewer.core().align_camera_center(V[ti], F[ti]);
                    ti++;
                }
            }
            
            if (ImGui::Button("Loop", ImVec2(-1, 0)))
            {

                OpenMesh::Subdivider::Uniform::LoopT<MyMesh> LoopT;

                if (ti < cache_size)
                {
                    if (!flag[ti])
                    {
                        // Execute  subdivision steps
                        LoopT.attach(mesh);
                        LoopT(1);
                        LoopT.detach();
                        std::cout << "vertices : " << mesh.n_vertices() << std::endl;;
                        std::cout << "faces : " << mesh.n_faces() << std::endl;;
                        openMesh_to_igl(mesh, V[ti], F[ti]);
                        flag[ti] = true;
                    }
                    viewer.data().clear();
                    viewer.data().set_mesh(V[ti], F[ti]);
                    viewer.core().align_camera_center(V[ti], F[ti]);
                    ti++;
                }
            }

            if (ImGui::Button("setTexture", ImVec2(-1, 0)))
            {

                viewer.data().set_uv(uv_coords[0]);
                /*size_t last_dot = file.rfind('.');
                std::string mtlpath = file.substr(0, (last_dot)) + ".mtl";

                std::ifstream mtlfile(mtlpath);
                std::string imgpath;
                std::string line;
                if (!mtlfile.is_open())
                {
                    std::cerr << "Error: Cannot open file " << mtlpath << std::endl;
                }
                while (std::getline(mtlfile, line))
                {
                    if (line.substr(0, 9) == "   map_Kd")
                    {
                        imgpath = line.substr(10);
                        std::cout << imgpath << "\n";
                    }
                }*/

                Eigen::MatrixXd colors = Eigen::MatrixXd::Ones(viewer.data().V.rows(), 3);
                viewer.data().set_colors(colors);
                Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> R, G, B, A;
                igl::png::readPNG(mtlfile.is_open() ? texture_file:igl::file_dialog_open(), R, G, B, A);
                viewer.data().set_mesh(V[0], F[0]);
                viewer.data().set_texture(R, G, B, A);
                viewer.data().show_texture = true;
                viewer.data().show_lines = false;
            }


            if (ImGui::Button("Simplify", ImVec2(-1, 0)))
            {
                if (ti < cache_size)
                {
                    if (!flag[ti])
                    {
                        uv_coords[ti] = Surface_Simplification(mesh, 0.5, uv_coords[0]);
                        std::cout << "vertices : " << mesh.n_vertices() << std::endl;;
                        std::cout << "faces : " << mesh.n_faces() << std::endl;;
                        openMesh_to_igl(mesh, V[ti], F[ti]);
                        flag[ti] = true;
                    }
                    viewer.data().clear();
                    viewer.data().set_mesh(V[ti], F[ti]);
                    viewer.data().set_uv(uv_coords[ti]);
                    Eigen::MatrixXd colors = Eigen::MatrixXd::Ones(viewer.data().V.rows(), 3);
                    viewer.data().set_colors(colors);
                    Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> R, G, B, A;
                    igl::png::readPNG(mtlfile.is_open() ? texture_file : igl::file_dialog_open(), R, G, B, A);
                    viewer.data().set_texture(R, G, B, A);
                    viewer.data().show_texture = true;
                    viewer.data().show_lines = false;
                    viewer.core().align_camera_center(V[ti], F[ti]);
                    ti++;
                }
            }

            ImGui::InputDouble("TargetLength", &TargetLength, 0, 0, "%.4f");
            if (ImGui::Button("Remesh", ImVec2(-1, 0)))
            {
                if (ti < cache_size)
                {
                    if (!flag[ti])
                    {
                        Remesh(mesh, TargetLength);
                        std::cout << "vertices : " << mesh.n_vertices() << std::endl;;
                        std::cout << "faces : " << mesh.n_faces() << std::endl;;
                        openMesh_to_igl(mesh, V[ti], F[ti]);
                        flag[ti] = true;
                    }
                    viewer.data().clear();
                    viewer.data().set_mesh(V[ti], F[ti]);
                    viewer.core().align_camera_center(V[ti], F[ti]);
                    ti++;
                }
            }
        }
    };

   
    
    viewer.data().set_mesh(V[0], F[0]);
    viewer.data().show_texture = true;
    viewer.data().show_lines = false;
    viewer.launch();

}

