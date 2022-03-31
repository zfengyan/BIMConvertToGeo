#include <iostream>
#include <fstream>

#include "json.hpp"
#include "Polyhedra.hpp"


void process_shell_explorer_indices(Shell_explorer& se);
void write_to_json(std::vector<Shell_explorer>& shell_explorers);


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
	std::vector<Shell_explorer> shell_explorers;
	
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
			
			//process the indices
			process_shell_explorer_indices(se);

			//add the se to shell_explorers
			shell_explorers.push_back(se);
		}
	}

	std::cout << "after extracting geometries: " << '\n';
	std::cout << shell_explorers.size() << '\n';
	std::cout << "-------------------------------" << '\n';
	for (auto& shell : shell_explorers) {
		std::cout << "vertices size of this shell: " << shell.vertices.size() << '\n';
		std::cout << "half facets size of this shell: " << shell.faces.size() << '\n';
		std::cout << '\n';
	}

	write_to_json(shell_explorers);

	return 0;
}


void process_shell_explorer_indices(Shell_explorer& se) {
	int accumulated_index = 0;
	for (auto& face : se.faces) {
		for (auto& index : face) {
			index = accumulated_index++;
		}
	}
}


void write_to_json(std::vector<Shell_explorer>& shell_explorers) {
	// basic info
	nlohmann::json json;
	json["type"] = "CityJSON";
	json["version"] = "1.1";
	json["transform"] = nlohmann::json::object();
	json["transform"]["scale"] = nlohmann::json::array({ 1.0, 1.0, 1.0 });
	json["transform"]["translate"] = nlohmann::json::array({ 0.0, 0.0, 0.0 });
	
	// Building info
	json["CityObjects"] = nlohmann::json::object();
	json["CityObjects"]["Building_1"]["type"] = "Building";
	json["CityObjects"]["Building_1"]["attributes"] = nlohmann::json({});
	json["CityObjects"]["Building_1"]["children"] = nlohmann::json::array({ "Building_1_0" });
	json["CityObjects"]["Building_1"]["geometry"] = nlohmann::json::array({});

	// BuildingPart - exterior?
	json["CityObjects"]["Building_1_0"]["type"] = "BuildingPart";
	json["CityObjects"]["Building_1_0"]["attributes"] = nlohmann::json({});
	json["CityObjects"]["Building_1_0"]["parents"] = nlohmann::json::array({"Building_1"});
	json["CityObjects"]["Building_1_0"]["geometry"] = nlohmann::json::array();
	json["CityObjects"]["Building_1_0"]["geometry"][0]["type"] = "Solid";
	json["CityObjects"]["Building_1_0"]["geometry"][0]["lod"] = "2.2";
	json["CityObjects"]["Building_1_0"]["geometry"][0]["boundaries"] = nlohmann::json::array({}); // indices
	json["vertices"] = nlohmann::json::array({}); // vertices

	auto const& se = shell_explorers[0];
	for (auto& v : se.vertices) {
		double x = CGAL::to_double(v.x()); // warning: may have precision loss
		double y = CGAL::to_double(v.y());
		double z = CGAL::to_double(v.z());
		json["vertices"].push_back({ x, y, z });		
	} // vertices may be repeated

	int boundaries_index = 0;
	std::vector<unsigned long> one_face;
	auto& b = json["CityObjects"]["Building_1_0"]["geometry"][0]["boundaries"];
	for (auto& face : se.faces) {
		std::vector<unsigned long> one_face;
		for (auto& index : face) {
			one_face.push_back(index);
		}
		b.push_back({ { one_face } });
		one_face.clear();
	}

	std::string json_string = json.dump(2);
	std::string outputname = "/mybuilding.json";
	std::ofstream out_stream(OUTPUT_PATH + outputname);
	out_stream << json_string;
	out_stream.close();
}