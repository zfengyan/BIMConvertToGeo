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
        std::cout << "building surface with " << vertices.size() << " vertices and " << faces.size() << " faces" << std::endl;

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

};