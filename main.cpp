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
#include"Patches.h"

typedef OpenMesh::TriMesh_ArrayKernelT<> MyMesh;
const int cache_size = 12;
Eigen::MatrixXd V[cache_size];
Eigen::MatrixXi F[cache_size];
bool flag[cache_size] = { false };
MyMesh mesh;
double TargetLength = 0.008;
float alfa = 0.5;
int index = 0;

int ti = 0;
bool key_down(igl::opengl::glfw::Viewer& viewer, unsigned char key, int modifier)
{
    if (key == '1')
    {
        if (ti < cache_size)
        {
            if (!flag[ti])
            {
                Surface_Simplification(mesh, alfa);
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
    std::string fname = "../models/arc.off";

    OpenMesh::IO::read_mesh(mesh, argc > 1 ? argv[1] : fname);

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

            

            // Add a button
            if (ImGui::Button("mesh_filter", ImVec2(-1, 0)))
            {

                if (ti < cache_size)
                {
                    if (!flag[ti])
                    {

                        mesh_filter(mesh, 1);

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

            ImGui::InputFloat("Simplify_ratio", &alfa, 0, 0, "%.4f");
            if (ImGui::Button("Simplify", ImVec2(-1, 0)))
            {
                if (ti < cache_size)
                {
                    if (!flag[ti])
                    {
                        auto start = std::chrono::steady_clock::now();
                        Surface_Simplification(mesh, alfa);
                        std::cout << "vertices : " << mesh.n_vertices() << std::endl;;
                        std::cout << "faces : " << mesh.n_faces() << std::endl;;
                        auto end = std::chrono::steady_clock::now();
                        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                        std::cout << "Simplify time: " << std::setprecision(3) << duration.count() / 1000.0 << " secs" << std::endl;
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
            ImGui::InputInt("vertex_id", &index, 0, 0);
            if (ImGui::Button("showVertex", ImVec2(-1, 0)))
            {
                Eigen::MatrixXd C;
                if (ti < cache_size)
                {
                    if (!flag[ti])
                    { 
                        C.resize(V[0].rows(), 3);
                        C.col(0).setConstant(0.8); 
                        C.col(1).setConstant(0.8);
                        C.col(2).setConstant(0.8);
                        /*for (auto index : tables)
                        {
                            C.row(index) << 0.0, 1.0, 0.0;
                        }
                        C.row(tables[0]) << 1.0, 0.0, 0.0;*/
                        C.row(index) << 0.0, 1.0, 0.0;
                        std::cout << "vertices : " << mesh.n_vertices() << std::endl;;
                        std::cout << "faces : " << mesh.n_faces() << std::endl;;
                        openMesh_to_igl(mesh, V[ti], F[ti]);
                        flag[ti] = true;
                    }
                    viewer.data().clear();
                    viewer.data().set_mesh(V[ti], F[ti]);
                    viewer.data().set_colors(C);
                    viewer.data().show_vertex_labels = true;
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
    viewer.core().background_color << 1.0, 1.0, 1.0, 1.0;
    //viewer.data().set_colors(Eigen::RowVector3d(0.8, 0.8, 0.8));
    viewer.launch();

}

