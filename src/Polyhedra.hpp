#pragma once

#include "LoadOBJ.hpp"

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Polyhedron_incremental_builder_3.h>
#include <CGAL/Nef_polyhedron_3.h>

typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;
typedef Kernel::Point_3 Point;
typedef CGAL::Polyhedron_3<Kernel> Polyhedron;
typedef CGAL::Nef_polyhedron_3<Kernel> Nef_polyhedron;

template <class HDS>
struct Polyhedron_builder : public CGAL::Modifier_base<HDS> {
    std::vector<Point> vertices; // type: Kernel::Point_3, for EACH SHELL
    std::vector<std::vector<unsigned long>> faces; // INDEX for vertices in EACH SHELL

    Polyhedron_builder() {}
    void operator()(HDS& hds) {
        CGAL::Polyhedron_incremental_builder_3<HDS> builder(hds, true);
        std::cout << "constructing polyhedron -- ";
        std::cout << "building surface with " << vertices.size() << " vertices and " << faces.size() << " faces" << '\n';

        builder.begin_surface(vertices.size(), faces.size());
        for (auto const& vertex : vertices) builder.add_vertex(vertex);
        for (auto const& face : faces) builder.add_facet(face.begin(), face.end());
        builder.end_surface();
    }
};

struct Nef {
    std::vector<Nef_polyhedron> nef_polyhedron_list; // store all Nef polyhedrons
};

class Construct_Nef_Polyhedron {
private:
    /*
    * check if a vertex already exists in a vertices vector
    * USE coordinates not indices to compare whether two vertices are the same
    * return: False - not exist, True - exist
    */
    static bool vertex_exist_check(std::vector<Point>& vertices, Point& vertex) {
        bool flag(false);
        for (auto& v : vertices) {
            if (
                abs(vertex.x() - v.x()) < Epsilon &&
                abs(vertex.y() - v.y()) < Epsilon &&
                abs(vertex.z() - v.z()) < Epsilon) {

                flag = true;
            }
        }
        return flag;
    }


    /*
    * if a vertex is repeated, find the index and return
    */
    static unsigned long find_vertex(std::vector<Point>& vertices, Point& vertex) {
        for (std::size_t i = 0; i != vertices.size(); ++i) {
            if (
                abs(vertex.x() - vertices[i].x()) < Epsilon &&
                abs(vertex.y() - vertices[i].y()) < Epsilon &&
                abs(vertex.z() - vertices[i].z()) < Epsilon) {

                return (unsigned long)i;
            }
        }
        std::cout << "warning: please check find_vertex function" << '\n';
        return 0;
    }
public:
    /*
    * construct nef polyhedrons and store to nef_polyhedron_list
    * OBJFile: use f.new_vertices, vertex.newid and face.new_v_indices
    */
    static void construct_nef_polyhedron(OBJFile& f, Nef& nef) {
        std::cout << '\n';
        std::cout << "-- constructing Nef polyhedron..." << '\n';

        unsigned long count = 0;
        for (auto& obj : f.objects) // access elements from objects
        {
            for (auto& shell : obj.shells) // load each shell into a CGAL Polyhedron_3 
            {
                Polyhedron polyhedron;
                Polyhedron_builder<Polyhedron::HalfedgeDS> polyhedron_builder;

                // add vertices of this shell to builder's vertices list
                for (auto const& v : shell.poly_vertices) {
                    polyhedron_builder.vertices.push_back(Point(v.x, v.y, v.z));
                }
                
                // add faces of this shell to builder's faces list
                for (auto const& face : shell.faces) {
                    polyhedron_builder.faces.emplace_back();
                    for (auto const& index : face.v_poly_indices) {
                        polyhedron_builder.faces.back().push_back(index); // index is 0-based
                    }
                }
                polyhedron.delegate(polyhedron_builder);
                ++count;

                // if polyhedron is closed, convert it to Nef_polyhedron and store
                if (polyhedron.is_closed()) {
                    Nef_polyhedron nef_polyhedron(polyhedron);
                    nef.nef_polyhedron_list.emplace_back(nef_polyhedron);
                }
            }
        }
        std::cout << "construct " << count << " polyhedrons " << "for " << f.shells.size() << " shells" << '\n';
        std::cout << "construct " << nef.nef_polyhedron_list.size() << " nef polyhedrons " << '\n';
    }


    /*
    * construct polyhedron for each obj shell
    */
    static void construct_polyhedron_each_shell_obj(std::string& fname) {
        std::string path = INTER_PATH;
        std::string filename = path + fname;
        std::cout << "-- loading obj shell: " << filename << '\n';

        // read obj
        std::string line;
        std::ifstream file(filename);
        if (!file.is_open()) { std::cerr << "file open failed! " << '\n'; }
        unsigned long vertex_index = 1; // track vertex index in obj file, starts from 1	   
        std::vector<double> coordinates; // store xyz coordinates of each vertex line
        std::vector<unsigned long> face_v_indices; // store face-vertex indices in each face line

        Polyhedron_builder<Polyhedron::HalfedgeDS> polyhedron_builder; // construct polyhedron_builder

        // process each line in the obj file
        while (std::getline(file, line)) {
            if (line.empty()) {
                continue; // skip empty lines:
            }

            // use markers below to mark the type of each line
            bool v_flag(false); // entitled "v"
            bool f_flag(false); // entitled "f"

            // help to process elements in each line
            std::stringstream ss(line);
            std::string field; // element in each line
            std::string::size_type sz; // NOT std::size_t, size of std::string

            // for each element(field, type: string) in one line
            while (std::getline(ss, field, ' ')) {

                // get the type of current line
                if (field == "f") {
                    f_flag = true;
                    continue;
                }
                else if (field == "v") {
                    v_flag = true;
                    continue;
                }

                // process the current line
                if (v_flag) {
                    // process xyz coordinates
                    coordinates.emplace_back(std::stod(field, &sz));
                }
                else if (f_flag) {
                    face_v_indices.emplace_back((unsigned long)std::stoi(field, &sz) - 1);
                }

            } // end while: process each element in one line

            // process each vertex (if it's a vertex line)
            if (!coordinates.empty() && coordinates.size() == 3) {
                polyhedron_builder.vertices.emplace_back(
                    Point(
                        coordinates[0], // x
                        coordinates[1], // y
                        coordinates[2]) // z
                );
                ++vertex_index; // vertex index in obj file
            }
            coordinates.clear();

            // constrcut face and add it into faces (if it's a face line)
            if (!face_v_indices.empty()) {
                polyhedron_builder.faces.emplace_back(face_v_indices);
            }
            face_v_indices.clear();

        } // end while: each line in the file

        // construct polyhedron ------------------------------------------------------------------
        Polyhedron polyhedron;
        polyhedron.delegate(polyhedron_builder);
        std::cout << "done" << '\n';
    }
};