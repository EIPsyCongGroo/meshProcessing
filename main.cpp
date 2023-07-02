#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Tools/Subdivider/Uniform/LoopT.hh>
#include <OpenMesh/Tools/Subdivider/Uniform/Sqrt3T.hh>

#include <igl/opengl/glfw/Viewer.h>
#include <igl/opengl/glfw/imgui/ImGuiPlugin.h>
#include <igl/opengl/glfw/imgui/ImGuiMenu.h>
#include <igl/opengl/glfw/imgui/ImGuiHelpers.h>
#include "Base.h"
#include "simplify.h"
#include "remesh.h"

typedef OpenMesh::TriMesh_ArrayKernelT<> MyMesh;
const int cache_size = 12;
Eigen::MatrixXd V[cache_size];
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
                Surface_Simplification(mesh, 0.5, alfa);
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
    mesh.add_property(test);
    mesh.request_vertex_normals();
    mesh.request_face_normals();

    mesh.request_vertex_texcoords2D();
    OpenMesh::IO::Options opt = OpenMesh::IO::Options::VertexTexCoord;
    std::string fname = "../models/bunny.obj";
    if (!(argv[1]))
    {
        OpenMesh::IO::read_mesh(mesh, fname, opt);
    }
    else
    {
        OpenMesh::IO::read_mesh(mesh, argv[1], opt);
    }
    for (MyMesh::VertexIter v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); ++v_it)
    {
        mesh.point(*v_it) /= 4.0;
    }
    mesh.update_face_normals();
    mesh.update_vertex_normals();
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

    // Customize the menu
    double doubleVariable = 0.1f; // Shared between two menus

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

            //if (ImGui::Button("CatmullClark", ImVec2(-1, 0)))
            //{

            //    OpenMesh::Subdivider::Uniform::CatmullClarkT<MyMesh> catmull;;

            //    if (ti < cache_size)
            //    {
            //        if (!flag[ti])
            //        {
            //            // Execute  subdivision steps
            //            catmull.attach(mesh);
            //            catmull(1);
            //            catmull.detach();
            //            std::cout << "vertices : " << mesh.n_vertices() << std::endl;;
            //            std::cout << "faces : " << mesh.n_faces() << std::endl;;
            //            openMesh_to_igl(mesh, V[ti], F[ti]);
            //            flag[ti] = true;
            //        }
            //        viewer.data().clear();
            //        viewer.data().set_mesh(V[ti], F[ti]);
            //        viewer.core().align_camera_center(V[ti], F[ti]);
            //        ti++;
            //    }
            //}

            // Add a button
            //if (ImGui::Button("Decimater", ImVec2(-1, 0)))
            //{

            //    typedef OpenMesh::Decimater::DecimaterT< MyMesh >     Decimater;
            //    typedef OpenMesh::Decimater::ModQuadricT<MyMesh>::Handle HModQuadric;


            //    if (ti < cache_size)
            //    {
            //        if (!flag[ti])
            //        {

            //            Decimater   decimater(mesh);  // a decimater object, connected to a mesh
            //            HModQuadric hModQuadric;      // use a quadric module
            //            decimater.add(hModQuadric); // register module at the decimater
            //            // Execute  decimation steps
            //            decimater.module(hModQuadric).unset_max_err();
            //            decimater.initialize();       // let the decimater initialize the mesh and the
            //            // modules
            //            decimater.decimate();         // do decimation

            //            std::cout << "vertices : " << mesh.n_vertices() << std::endl;;
            //            std::cout << "faces : " << mesh.n_faces() << std::endl;;
            //            openMesh_to_igl(mesh, V[ti], F[ti]);
            //            flag[ti] = true;
            //        }
            //        viewer.data().clear();
            //        viewer.data().set_mesh(V[ti], F[ti]);
            //        viewer.core().align_camera_center(V[ti], F[ti]);
            //        ti++;
            //    }
            //}

            ImGui::InputFloat("Curvature weight", &alfa, 0, 0, "%.4f");
            if (ImGui::Button("Simplify", ImVec2(-1, 0)))
            {
                if (ti < cache_size)
                {
                    if (!flag[ti])
                    {
                        Surface_Simplification(mesh, 0.5, alfa);
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

    // Draw additional windows
    //menu.callback_draw_custom_window = [&]()
    //{
    //    // Define next window position + size
    //    ImGui::SetNextWindowPos(ImVec2(180.f * menu.menu_scaling(), 10), ImGuiCond_FirstUseEver);
    //    ImGui::SetNextWindowSize(ImVec2(200, 160), ImGuiCond_FirstUseEver);
    //    ImGui::Begin(
    //        "File Info", nullptr,
    //        ImGuiWindowFlags_NoSavedSettings
    //    );

    //    // Expose the same variable directly ...
    //    ImGui::PushItemWidth(-80);
    //    ImGui::DragScalar("double", ImGuiDataType_Double, &doubleVariable, 0.1, 0, 0, "%.4f");
    //    ImGui::PopItemWidth();

    //    static std::string str = argv[1];
    //    size_t last_dot = str.rfind('.');
    //    size_t last_slash = str.find_last_of('\\');
    //    std::string fnameNew = str.substr(last_slash + 1, (last_dot - last_slash - 1));
    //    
    //    ImGui::InputText("fname", fnameNew);

    //    ImGui::End();
    //};
    
    viewer.data().set_mesh(V[0], F[0]);

    // Disable wireframe
    //viewer.data().show_lines = false;
    //viewer.data().show_texture = true;
    viewer.launch();

}

