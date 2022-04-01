#pragma once


#include "LoadOBJ.hpp"


#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Polyhedron_incremental_builder_3.h>
#include <CGAL/Nef_polyhedron_3.h>
#include <CGAL/convex_hull_3.h>
#include <CGAL/minkowski_sum_3.h>


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
        std::cout << "-- building surface with " << vertices.size() << " vertices and " << faces.size() << " faces" << '\n';

        builder.begin_surface(vertices.size(), faces.size());
        for (auto const& vertex : vertices) builder.add_vertex(vertex);
        for (auto const& face : faces) builder.add_facet(face.begin(), face.end());
        builder.end_surface();
    }
};


// help to store the nef_polyhedron_list
struct Nef {
    std::vector<Nef_polyhedron> nef_polyhedron_list; // store all Nef polyhedrons
    Nef_polyhedron big_nef; // store the big nef
};


// build nef polyhedra from(polyhedron builder and convex hull)
class Build_Nef_Polyhedron {
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
    * compute convex hull for shells which don't work for polyhedron builder
    */
    static Nef_polyhedron build_convexhull(std::string& fname) {
        std::string path = INTER_PATH;
        std::string filename = path + fname;
        std::cout << "reading obj shell: " << fname;

        // read obj
        std::string line;
        std::ifstream file(filename);
        if (!file.is_open()) { std::cerr << "file open failed! " << '\n'; }
     
        std::vector<double> coordinates; // store xyz coordinates of each vertex line
        std::vector<Point> vertices;

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

            } // end while: process each element in one line

            // process each vertex (if it's a vertex line)
            if (!coordinates.empty() && coordinates.size() == 3) {
                vertices.emplace_back(
                    Point(
                        coordinates[0], // x
                        coordinates[1], // y
                        coordinates[2]) // z
                );
            }
            coordinates.clear();

        } // end while: each line in the file

        // use Point_3 points in vertices to compute the convex hull
        
        // define polyhedron to hold convex hull
        Polyhedron poly;
        // compute convex hull of non-collinear points
        CGAL::convex_hull_3(vertices.begin(), vertices.end(), poly);
        //std::cout << "The convex hull contains " << poly.size_of_vertices() << " vertices" << '\n';
        if (poly.is_closed()) {
            // convert the poly to nef_poly and add it to nef
            std::cout << " build convex hull " << '\n';
            Nef_polyhedron nef_poly(poly);
            return nef_poly;
        }

    }


    /*
    * construct polyhedron for each obj shell
    */
    static Nef_polyhedron build_polyhedron_each_shell(std::string& fname) {
        std::string path = INTER_PATH;
        std::string filename = path + fname;
        std::cout << "reading obj shell: " << fname << " ";

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

        // construct polyhedron and add it to nef ------------------------------------------------------------------
        Polyhedron polyhedron;
        polyhedron.delegate(polyhedron_builder);
        if (polyhedron.is_closed()) {
            std::cout << "build nef polyhedron " << '\n';
            Nef_polyhedron nef_poly(polyhedron);
            return nef_poly;
        }
       
    }


    /*
    * build polyhedra from polyhedron builder and convexhull
    * for 1~17.obj files, use polyhedron builder to build polyhedra
    * for 18~33.obj files, use the corresponding convex hull to build polyhedra and store the polyhedra as .off files
    */
    static void build_nef_polyhedra(Nef& nef) {

        // common file prefix and suffix for obj files
        std::string prefix = "/";
        std::string suffix_obj = ".obj";
     
        std::cout << "-- reading 1.obj to 17.obj, these shells can be passed to polyhedron builder" << '\n';
        std::cout << "-- reading 18.obj to 33.obj, use these shells' convex hull to build corresponding polyhedron" << '\n';

        // from 1.obj to 17.obj
        for (int shell_id = 1; shell_id != 18; ++shell_id) {
            //if (shell_id == 5)continue;
            if ( shell_id == 2 || shell_id == 12 || shell_id == 3 ||
                shell_id == 4  || shell_id == 14 || shell_id == 1 || shell_id == 1) {
                std::string shell_str = std::to_string(shell_id);
                std::string shell_name = prefix + shell_str + suffix_obj;
                Nef_polyhedron nef_poly = build_convexhull(shell_name);
                nef.nef_polyhedron_list.push_back(nef_poly);
                
            }
            else if(shell_id == 16 || shell_id == 17){
                std::string shell_str = std::to_string(shell_id);
                std::string shell_name = prefix + shell_str + suffix_obj;
                Nef_polyhedron nef_poly = build_polyhedron_each_shell(shell_name);            
                
                std::string fcube = "/cube1.obj"; // used for miniskow_sum
                //Nef_polyhedron nef_poly_miniskow = miniskow_sum(fcube, nef_poly);
                nef.nef_polyhedron_list.push_back(nef_poly);
            }
            else {
                std::string shell_str = std::to_string(shell_id);
                std::string shell_name = prefix + shell_str + suffix_obj;
                Nef_polyhedron nef_poly = build_polyhedron_each_shell(shell_name);
                nef.nef_polyhedron_list.push_back(nef_poly);
            }           
        }

        // from 18.obj to 33.obj
        for (int shell_id = 18; shell_id != 34; ++shell_id) {
            if (shell_id == 21 || shell_id == 22 || shell_id == 19 || shell_id == 32 ||
                shell_id == 23 || shell_id == 25 || shell_id == 26 || shell_id == 27 ||
                shell_id == 28 || shell_id == 18 || shell_id == 33 || shell_id == 20 || shell_id == 24)continue;

            std::string shell_str = std::to_string(shell_id);
            std::string shell_name = prefix + shell_str + suffix_obj;

            Nef_polyhedron nef_poly = build_convexhull(shell_name);
            nef.nef_polyhedron_list.push_back(nef_poly);        
        }
      
        // output nef_polyhedron_list size
        std::cout << "build " << nef.nef_polyhedron_list.size() << " " << "Nef polyhedra" << '\n';
    }


    static Nef_polyhedron miniskow_sum(std::string& fcube, Nef_polyhedron& nef_polyhedron) {
        std::string path = INTER_PATH;
        std::string filename = path + fcube;

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

        // construct polyhedron and convert it to nef ------------------------------------------------------------------
        Polyhedron polyhedron;
        polyhedron.delegate(polyhedron_builder);
        if (polyhedron.is_closed()) {
            Nef_polyhedron nef_cube(polyhedron);
            Nef_polyhedron result = CGAL::minkowski_sum_3(nef_polyhedron, nef_cube);
            return result;
        }

    }
};


// use CSG to build big Nef polyhedron
class BigNef {
public:
    static void test_big(Nef& nef) {
		
		for (auto& one_nef : nef.nef_polyhedron_list) {
            nef.big_nef += one_nef;
		}
		std::cout << "is simple: " << nef.big_nef.is_simple() << '\n';
		std::cout << "num of vertices of the Nef after operation: " << nef.big_nef.number_of_vertices() << '\n';
        
    }


};


// extract geometries
struct Shell_explorer {
    std::vector<Point> vertices;
    std::vector<std::vector<unsigned long>> faces;

    void visit(Nef_polyhedron::Vertex_const_handle v) {}
    void visit(Nef_polyhedron::Halfedge_const_handle he) {
        /*
        Nef_polyhedron::Vertex_const_handle v_source = he->source();
        Nef_polyhedron::Vertex_const_handle v_target = he->target();
        std::cout << "source: ";
        vertices.emplace_back(v_source->point());
        //std::cout << "(" << v_source->point().x() << ", " << v_source->point().y() << ", " << v_source->point().z() << ")" << " ";
        std::cout << "target: ";
        //std::cout << "(" << v_target->point().x() << ", " << v_target->point().y() << ", " << v_target->point().z() << ")" << '\n';
        vertices.emplace_back(v_target->point());
        */
    }
    void visit(Nef_polyhedron::SHalfedge_const_handle she) {}
    void visit(Nef_polyhedron::SHalfloop_const_handle shl) {}
    void visit(Nef_polyhedron::SFace_const_handle sf) {}

    void visit(Nef_polyhedron::Halffacet_const_handle hf) {
        for (Nef_polyhedron::Halffacet_cycle_const_iterator it = hf->facet_cycles_begin(); it != hf->facet_cycles_end(); it++) {
            
            //std::cout << it.is_shalfedge() << " " << it.is_shalfloop() << '\n';
            Nef_polyhedron::SHalfedge_const_handle she = Nef_polyhedron::SHalfedge_const_handle(it);
            CGAL_assertion(she != 0);
            Nef_polyhedron::SHalfedge_around_facet_const_circulator hc_start = she;
            Nef_polyhedron::SHalfedge_around_facet_const_circulator hc_end = hc_start;
            //std::cout << "hc_start = hc_end? " << (hc_start == hc_end) << '\n';

            faces.emplace_back();
            int index = 0;
            CGAL_For_all(hc_start, hc_end) // each vertex of one halffacet
            {
                Nef_polyhedron::SVertex_const_handle svert = hc_start->source();
                Point vpoint = svert->center_vertex()->point();
                //std::cout << "v: " << "(" << vpoint.x() << ", " << vpoint.y() << ", " << vpoint.z() << ")" << '\n';
                vertices.push_back(vpoint);
                faces.back().push_back(index++);
            }
            //std::cout << '\n';
         
        }

    }
};

