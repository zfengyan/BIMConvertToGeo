#include <iostream>

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
	// PreparePolyhedron::prepare_poly_vertices_face_indices(f); // uncomment this to output each shell
	// PreparePolyhedron::output_each_shell(f); // uncomment this to output each shell

	std::cout << '\n';
	std::string obj_name = "/1.obj";
	Construct_Nef_Polyhedron::construct_polyhedron_each_shell_obj(obj_name);
	
	// Nef nef;
	// Construct_Nef_Polyhedron::construct_nef_polyhedron(f, nef);
	

	return 0;
}
