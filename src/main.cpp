#include <iostream>
#include <fstream>

#include "json.hpp"
#include "Polyhedra.hpp"

int main()
{
	//std::cout << "Hello CMake." << '\n';
	std::cout << "-- activated data folder: " << DATA_PATH << '\n';

	//OBJFile f; // organize vertcies, faces, shells and objects
	//
	//std::cout << '\n';
	//std::string fname = "/KIT.obj";
	//LoadOBJ::load_obj(fname, f);

	//std::cout << '\n';
	//std::string repeated_info_name = "/KIT.repeated.vertices.txt";
	//LoadOBJ::repeated_vertices_info(repeated_info_name, f);

	//std::cout << '\n';
	//std::string repeated_faces_name = "/KIT.repeated.faces.txt";
	//LoadOBJ::repeated_faces_info(repeated_faces_name, f);

	//std::cout << '\n';
	//std::string new_vertices_name = "/KIT.new.vertices.txt";
	//LoadOBJ::process_repeated_vertices(new_vertices_name, f);

	//std::cout << '\n';
	//std::string new_faces_name = "/KIT.new.faces.txt";
	//LoadOBJ::process_repeated_faces(new_faces_name, f);

	//std::cout << '\n';
	//std::string output_obj_name = "/KIT.output.obj";
	//LoadOBJ::output_obj(output_obj_name, f);

	/* 
	* prepare the verticesand face - indices for each shell
	* shell.poly_vertices -- store the vertices of this shell
	* shell.faces -> face.v_poly_indices -- store the indices(0-based) point to the shell.poly_vertices 
	*/
	//PreparePolyhedron::prepare_poly_vertices_face_indices(f); // uncomment this to output each shell
	//PreparePolyhedron::output_each_shell(f); // uncomment this to output each shell

	std::cout << '\n';
	
	Nef nef;

	// test
	// std::string obj_name = "/20.obj";
	// Build_Nef_Polyhedron::build_polyhedron_each_shell(obj_name, nef);
	// Build_Nef_Polyhedron::build_convexhull(obj_name, nef);

	// build Nef_polyhedra according to different shells
	Build_Nef_Polyhedron::build_nef_polyhedra(nef);

	// build big Nef
	BigNef::test_big(nef);
	
	// extract geometries ------------------------------------------------------------
	
	
	int volume_count = 0;
	int shell_count = 0;
	Nef_polyhedron::Volume_const_iterator current_volume;
	CGAL_forall_volumes(current_volume, nef.big_nef) {
		std::cout << "volume: " << volume_count++ << " ";
		std::cout << "volume mark: " << current_volume->mark() << '\n';
		Nef_polyhedron::Shell_entry_const_iterator current_shell;
		CGAL_forall_shells_of(current_shell, current_volume) {
			//std::cout << "shell: " << shell_count++ << '\n';
			Shell_explorer se;
			Nef_polyhedron::SFace_const_handle sface_in_shell(current_shell);
			nef.big_nef.visit_shell_objects(sface_in_shell, se);
			//...
		}
	}
	

	return 0;
}
