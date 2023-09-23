#include <donut/File.hpp>
#include <donut/Filesystem.hpp>
#include <donut/graphics/Error.hpp>
#include <donut/graphics/Image.hpp>
#include <donut/graphics/Mesh.hpp>
#include <donut/graphics/Model.hpp>
#include <donut/graphics/Texture.hpp>
#include <donut/math.hpp>
#include <donut/obj.hpp>

#include <algorithm>     // std::find_if
#include <array>         // std::array
#include <cstddef>       // std::size_t, std::byte
#include <exception>     // std::exception
#include <fmt/format.h>  // fmt::format
#include <functional>    // std::hash
#include <numbers>       // std::numbers_pi_v
#include <span>          // std::span
#include <string>        // std::string
#include <unordered_map> // std::unordered_map
#include <utility>       // std::move
#include <vector>        // std::vector

namespace donut::graphics {

namespace {

void generateNormals(std::span<Model::Object::Vertex> vertices, std::span<const Model::Object::Index> indices) {
	for (Model::Object::Vertex& vertex : vertices) {
		vertex.normal = {0.0f, 0.0f, 0.0f};
	}

	for (std::size_t triangleIndex = 0; triangleIndex + 3 <= indices.size(); triangleIndex += 3) {
		const std::size_t indexA = static_cast<std::size_t>(indices[triangleIndex]);
		const std::size_t indexB = static_cast<std::size_t>(indices[triangleIndex + 1]);
		const std::size_t indexC = static_cast<std::size_t>(indices[triangleIndex + 2]);

		const vec3 ab = vertices[indexB].position - vertices[indexA].position;
		const vec3 ac = vertices[indexC].position - vertices[indexA].position;
		const vec3 bc = vertices[indexC].position - vertices[indexB].position;

		const vec3 normal = cross(ab, ac);

		const float squareLengthAB = length2(ab);
		const float squareLengthAC = length2(ac);
		const float squareLengthBC = length2(bc);

		const float lengthAB = (squareLengthAB >= 1e-6f) ? sqrt(squareLengthAB) : 1e-3f;
		const float lengthAC = (squareLengthAC >= 1e-6f) ? sqrt(squareLengthAC) : 1e-3f;
		const float lengthBC = (squareLengthBC >= 1e-6f) ? sqrt(squareLengthBC) : 1e-3f;

		const float influenceAB = dot(ab, ac) / (lengthAB * lengthAC);
		const float influenceAC = -dot(ab, bc) / (lengthAB * lengthBC);
		const float influenceBC = dot(ac, bc) / (lengthAC * lengthBC);

		const float angleAB = (influenceAB >= 1.0f) ? 0.0f : (influenceAB <= -1.0f) ? std::numbers::pi_v<float> : acos(influenceAB);
		const float angleAC = (influenceAC >= 1.0f) ? 0.0f : (influenceAC <= -1.0f) ? std::numbers::pi_v<float> : acos(influenceAC);
		const float angleBC = (influenceBC >= 1.0f) ? 0.0f : (influenceBC <= -1.0f) ? std::numbers::pi_v<float> : acos(influenceBC);

		vertices[indexA].normal += normal * angleAB;
		vertices[indexB].normal += normal * angleAC;
		vertices[indexC].normal += normal * angleBC;
	}

	for (Model::Object::Vertex& vertex : vertices) {
		vertex.normal = normalize(vertex.normal);
	}
}

void generateTangentSpace(std::span<Model::Object::Vertex> vertices, std::span<const Model::Object::Index> indices) {
	for (Model::Object::Vertex& vertex : vertices) {
		vertex.tangent = {0.0f, 0.0f, 0.0f};
		vertex.bitangent = {0.0f, 0.0f, 0.0f};
	}

	for (std::size_t triangleIndex = 0; triangleIndex + 3 <= indices.size(); triangleIndex += 3) {
		const std::size_t indexA = static_cast<std::size_t>(indices[triangleIndex]);
		const std::size_t indexB = static_cast<std::size_t>(indices[triangleIndex + 1]);
		const std::size_t indexC = static_cast<std::size_t>(indices[triangleIndex + 2]);

		const vec3 ab = vertices[indexB].position - vertices[indexA].position;
		const vec3 ac = vertices[indexC].position - vertices[indexA].position;

		vec2 uvAB = vertices[indexB].textureCoordinates - vertices[indexA].textureCoordinates;
		vec2 uvAC = vertices[indexC].textureCoordinates - vertices[indexA].textureCoordinates;

		if (uvAB == vec2{0.0f, 0.0f} && uvAC == vec2{0.0f, 0.0f}) {
			uvAB = ab;
			uvAC = ac;
		}

		const float r = 1.0f / (uvAB.x * uvAC.y - uvAB.y * uvAC.x);
		const vec3 tangent = r * (ab * uvAC.y - ac * uvAB.y);

		vertices[indexA].tangent += tangent;
		vertices[indexB].tangent += tangent;
		vertices[indexC].tangent += tangent;
	}

	for (Model::Object::Vertex& vertex : vertices) {
		vertex.tangent = normalize(vertex.tangent - dot(vertex.tangent, vertex.normal) * vertex.normal);
		vertex.bitangent = cross(vertex.normal, vertex.tangent);
	}
}

[[nodiscard]] Texture loadTexture(const Filesystem& filesystem, const std::string& filepath) {
	return Texture{Image{filesystem, filepath.c_str(), {.highDynamicRange = filepath.ends_with(".hdr")}}};
}

void loadObjScene(Model& output, const Filesystem& filesystem, const char* filepath) {
	const obj::Scene scene = obj::Scene::parse(filesystem.openFile(filepath).readAllIntoString());

	std::string filepathPrefix = filepath;
	if (const std::size_t filepathLastSlashPosition = filepathPrefix.rfind('/'); filepathLastSlashPosition != std::string::npos) {
		filepathPrefix.erase(filepathLastSlashPosition + 1, std::string::npos);
	} else {
		filepathPrefix.clear();
	}

	std::vector<obj::mtl::Library> materialLibraries{};
	materialLibraries.reserve(scene.materialLibraryFilenames.size());
	for (const std::string& materialLibraryFilename : scene.materialLibraryFilenames) {
		materialLibraries.push_back(obj::mtl::Library::parse(filesystem.openFile((filepathPrefix + materialLibraryFilename).c_str()).readAllIntoString()));
	}

	struct FaceVertexHash {
		[[nodiscard]] std::size_t operator()(const obj::FaceVertex& faceVertex) const {
			return (std::hash<std::uint32_t>{}(faceVertex.vertexIndex) ^ (std::hash<std::uint32_t>{}(faceVertex.textureCoordinateIndex) << 1) >> 1) ^
			       (std::hash<std::uint32_t>{}(faceVertex.normalIndex) << 1);
		}
	};

	struct FaceVertexEqual {
		[[nodiscard]] bool operator()(const obj::FaceVertex& a, const obj::FaceVertex& b) const {
			return a.vertexIndex == b.vertexIndex && a.textureCoordinateIndex == b.textureCoordinateIndex && a.normalIndex == b.normalIndex;
		}
	};

	std::unordered_map<obj::FaceVertex, std::size_t, FaceVertexHash, FaceVertexEqual> vertexMap{};

	output.objects.reserve(scene.objects.size());
	for (const obj::Object& object : scene.objects) {
		for (const obj::Group& group : object.groups) {
			std::vector<Model::Object::Vertex> vertices{};
			std::vector<Model::Object::Index> indices{};

			vertexMap.clear();

			for (const obj::Face& face : group.faces) {
				if (face.vertices.size() >= 3) {
					for (std::size_t faceVertexIndex = 1; faceVertexIndex + 1 < face.vertices.size(); ++faceVertexIndex) {
						for (const std::size_t faceVertexIndex : {std::size_t{0}, faceVertexIndex, faceVertexIndex + 1}) {
							const obj::FaceVertex& faceVertex = face.vertices[faceVertexIndex];
							std::size_t vertexIndex = vertices.size();
							if (const auto [it, inserted] = vertexMap.emplace(faceVertex, vertexIndex); inserted) {
								vertices.push_back({
									.position = (faceVertex.vertexIndex < scene.vertices.size()) ? scene.vertices[faceVertex.vertexIndex] : vec3{0.0f, 0.0f, 0.0f},
									.normal = (faceVertex.normalIndex < scene.normals.size()) ? scene.normals[faceVertex.normalIndex] : vec3{0.0f, 0.0f, 0.0f},
									.tangent{},
									.bitangent{},
									.textureCoordinates = (faceVertex.textureCoordinateIndex < scene.textureCoordinates.size())
								                              ? scene.textureCoordinates[faceVertex.textureCoordinateIndex]
								                              : vec2{0.0f, 0.0f},
								});
							} else {
								vertexIndex = it->second;
							}
							indices.push_back(static_cast<Model::Object::Index>(vertexIndex));
						}
					}
				}
			}

			if (scene.normals.size() != scene.vertices.size()) {
				generateNormals(vertices, indices);
			}
			generateTangentSpace(vertices, indices);

			Model::Object::Material groupMaterial{
				.diffuseMap{},
				.specularMap{},
				.normalMap{},
				.emissiveMap{},
				.diffuseColor{1.0f, 1.0f, 1.0f},
				.specularColor{1.0f, 1.0f, 1.0f},
				.normalScale{1.0f, 1.0f, 1.0f},
				.emissiveColor{0.0f, 0.0f, 0.0f},
				.specularExponent = 1.0f,
				.dissolveFactor = 0.0f,
				.occlusionFactor = 1.0f,
			};
			if (!group.materialName.empty()) {
				for (const obj::mtl::Library& materialLibrary : materialLibraries) {
					if (const auto it = std::find_if(materialLibrary.materials.begin(), materialLibrary.materials.end(),
							[&](const obj::mtl::Material& material) -> bool { return material.name == group.materialName; });
						it != materialLibrary.materials.end()) {
						const obj::mtl::Material& material = *it;
						if (!material.diffuseMapName.empty()) {
							groupMaterial.diffuseMap = loadTexture(filesystem, filepathPrefix + material.diffuseMapName);
						}
						if (!material.specularMapName.empty()) {
							groupMaterial.specularMap = loadTexture(filesystem, filepathPrefix + material.specularMapName);
						}
						if (!material.bumpMapName.empty()) {
							groupMaterial.normalMap = loadTexture(filesystem, filepathPrefix + material.bumpMapName);
						}
						if (!material.emissiveMapName.empty()) {
							groupMaterial.emissiveMap = loadTexture(filesystem, filepathPrefix + material.emissiveMapName);
						}
						groupMaterial.diffuseColor = material.diffuseColor;
						groupMaterial.specularColor = material.specularColor;
						groupMaterial.emissiveColor = material.emissiveColor;
						groupMaterial.specularExponent = material.specularExponent;
						groupMaterial.dissolveFactor = material.dissolveFactor;
						groupMaterial.occlusionFactor = material.ambientColor.x * material.ambientColor.y * material.ambientColor.z;
						break;
					}
				}
			}

			output.objects.push_back({
				.mesh{Model::Object::VERTICES_USAGE, Model::Object::INDICES_USAGE, Model::Object::INSTANCES_USAGE, vertices, indices, {}},
				.material = std::move(groupMaterial),
				.indexCount = indices.size(),
			});
		}
	}
}

std::size_t sharedModelReferenceCount = 0;
alignas(Model) std::array<std::byte, sizeof(Model)> sharedQuadModelStorage;
alignas(Model) std::array<std::byte, sizeof(Model)> sharedCubeModelStorage;

} // namespace

const Model* const Model::QUAD = reinterpret_cast<Model*>(sharedQuadModelStorage.data());
const Model* const Model::CUBE = reinterpret_cast<Model*>(sharedCubeModelStorage.data());

Model::Model(const Filesystem& filesystem, const char* filepath) {
	try {
		loadObjScene(*this, filesystem, filepath);
	} catch (const obj::Error& e) {
		throw Error{fmt::format("Failed to load model \"{}\": Line {}: {}", filepath, e.lineNumber, e.what())};
	} catch (const std::exception& e) {
		throw Error{fmt::format("Failed to load model \"{}\": {}", filepath, e.what())};
	} catch (...) {
		throw Error{fmt::format("Failed to load model \"{}\".", filepath)};
	}
}

void Model::createSharedModels() {
	if (sharedModelReferenceCount == 0) {
		// clang-format off
		constexpr std::array QUAD_VERTICES{
			Object::Vertex{.position{-1.0f, -1.0f, -0.0f}, .normal{-0.0f, -0.0f, 1.0f}, .tangent{1.0f, 0.0f, 0.0f}, .bitangent{-0.0f, 1.0f, 0.0f}, .textureCoordinates{0.0f, 0.0f}},
			Object::Vertex{.position{1.0f, -1.0f, -0.0f}, .normal{-0.0f, -0.0f, 1.0f}, .tangent{1.0f, 0.0f, 0.0f}, .bitangent{-0.0f, 1.0f, 0.0f}, .textureCoordinates{1.0f, 0.0f}},
			Object::Vertex{.position{-1.0f, 1.0f, 0.0f}, .normal{-0.0f, -0.0f, 1.0f}, .tangent{1.0f, 0.0f, 0.0f}, .bitangent{-0.0f, 1.0f, 0.0f}, .textureCoordinates{0.0f, 1.0f}},
			Object::Vertex{.position{1.0f, 1.0f, 0.0f}, .normal{-0.0f, -0.0f, 1.0f}, .tangent{1.0f, 0.0f, 0.0f}, .bitangent{-0.0f, 1.0f, 0.0f}, .textureCoordinates{1.0f, 1.0f}},
		};
		constexpr std::array QUAD_INDICES{
			Object::Index{1}, Object::Index{2}, Object::Index{0}, Object::Index{1}, Object::Index{3}, Object::Index{2},
		};
		constexpr std::array CUBE_VERTICES{
			Object::Vertex{.position{-1.0f, -1.0f, 1.0f},  .normal{-0.0f, -1.0f, -0.0f}, .tangent{1.0f,  0.0f, 0.0f}, .bitangent{0.0f,  0.0f,  1.0f},  .textureCoordinates{0.125f, 0.75f}},
			Object::Vertex{.position{-1.0f, 1.0f,  1.0f},  .normal{-0.0f, 1.0f,  -0.0f}, .tangent{-1.0f, 0.0f, 0.0f}, .bitangent{0.0f,  0.0f,  1.0f},  .textureCoordinates{0.875f, 0.75f}},
			Object::Vertex{.position{-1.0f, -1.0f, -1.0f}, .normal{-0.0f, -1.0f, -0.0f}, .tangent{1.0f,  0.0f, 0.0f}, .bitangent{0.0f,  0.0f,  1.0f},  .textureCoordinates{0.125f, 0.5f}},
			Object::Vertex{.position{-1.0f, 1.0f,  -1.0f}, .normal{-0.0f, 1.0f,  -0.0f}, .tangent{-1.0f, 0.0f, 0.0f}, .bitangent{0.0f,  0.0f,  1.0f},  .textureCoordinates{0.875f, 0.5f}},
			Object::Vertex{.position{1.0f,  -1.0f, 1.0f},  .normal{-0.0f, -1.0f, -0.0f}, .tangent{1.0f,  0.0f, 0.0f}, .bitangent{0.0f,  0.0f,  1.0f},  .textureCoordinates{0.375f, 0.75f}},
			Object::Vertex{.position{1.0f,  1.0f,  1.0f},  .normal{-0.0f, 1.0f,  -0.0f}, .tangent{-1.0f, 0.0f, 0.0f}, .bitangent{0.0f,  0.0f,  1.0f},  .textureCoordinates{0.625f, 0.75f}},
			Object::Vertex{.position{1.0f,  -1.0f, -1.0f}, .normal{-0.0f, -1.0f, -0.0f}, .tangent{1.0f,  0.0f, 0.0f}, .bitangent{0.0f,  0.0f,  1.0f},  .textureCoordinates{0.375f, 0.5f}},
			Object::Vertex{.position{1.0f,  1.0f,  -1.0f}, .normal{-0.0f, 1.0f,  -0.0f}, .tangent{-1.0f, 0.0f, 0.0f}, .bitangent{0.0f,  0.0f,  1.0f},  .textureCoordinates{0.625f, 0.5f}},
			Object::Vertex{.position{-1.0f, -1.0f, -1.0f}, .normal{-1.0f, -0.0f, -0.0f}, .tangent{0.0f,  1.0f, 0.0f}, .bitangent{0.0f,  0.0f,  -1.0f}, .textureCoordinates{0.375f, 0.25f}},
			Object::Vertex{.position{-1.0f, -1.0f, -1.0f}, .normal{-0.0f, -0.0f, -1.0f}, .tangent{0.0f,  1.0f, 0.0f}, .bitangent{1.0f,  0.0f,  0.0f},  .textureCoordinates{0.375f, 0.25f}},
			Object::Vertex{.position{-1.0f, -1.0f, 1.0f},  .normal{-1.0f, -0.0f, -0.0f}, .tangent{0.0f,  1.0f, 0.0f}, .bitangent{0.0f,  0.0f,  -1.0f}, .textureCoordinates{0.375f, 0.0f}},
			Object::Vertex{.position{-1.0f, -1.0f, 1.0f},  .normal{-0.0f, -0.0f, 1.0f},  .tangent{0.0f,  1.0f, 0.0f}, .bitangent{-1.0f, 0.0f,  0.0f},  .textureCoordinates{0.375f, 1.0f}},
			Object::Vertex{.position{-1.0f, 1.0f,  1.0f},  .normal{-1.0f, -0.0f, -0.0f}, .tangent{0.0f,  1.0f, 0.0f}, .bitangent{0.0f,  0.0f,  -1.0f}, .textureCoordinates{0.625f, 0.0f}},
			Object::Vertex{.position{-1.0f, 1.0f,  1.0f},  .normal{-0.0f, -0.0f, 1.0f},  .tangent{0.0f,  1.0f, 0.0f}, .bitangent{-1.0f, 0.0f,  0.0f},  .textureCoordinates{0.625f, 1.0f}},
			Object::Vertex{.position{-1.0f, 1.0f,  -1.0f}, .normal{-1.0f, -0.0f, -0.0f}, .tangent{0.0f,  1.0f, 0.0f}, .bitangent{0.0f,  0.0f,  -1.0f}, .textureCoordinates{0.625f, 0.25f}},
			Object::Vertex{.position{-1.0f, 1.0f,  -1.0f}, .normal{-0.0f, -0.0f, -1.0f}, .tangent{0.0f,  1.0f, 0.0f}, .bitangent{1.0f,  0.0f,  0.0f},  .textureCoordinates{0.625f, 0.25f}},
			Object::Vertex{.position{1.0f,  -1.0f, -1.0f}, .normal{-0.0f, -0.0f, -1.0f}, .tangent{0.0f,  1.0f, 0.0f}, .bitangent{1.0f,  0.0f,  0.0f},  .textureCoordinates{0.375f, 0.5f}},
			Object::Vertex{.position{1.0f,  -1.0f, -1.0f}, .normal{1.0f,  -0.0f, -0.0f}, .tangent{0.0f,  1.0f, 0.0f}, .bitangent{0.0f,  -0.0f, 1.0f},  .textureCoordinates{0.375f, 0.5f}},
			Object::Vertex{.position{1.0f,  1.0f,  -1.0f}, .normal{-0.0f, -0.0f, -1.0f}, .tangent{0.0f,  1.0f, 0.0f}, .bitangent{1.0f,  0.0f,  0.0f},  .textureCoordinates{0.625f, 0.5f}},
			Object::Vertex{.position{1.0f,  1.0f,  -1.0f}, .normal{1.0f,  -0.0f, -0.0f}, .tangent{0.0f,  1.0f, 0.0f}, .bitangent{0.0f,  -0.0f, 1.0f},  .textureCoordinates{0.625f, 0.5f}},
			Object::Vertex{.position{1.0f,  -1.0f, 1.0f},  .normal{1.0f,  -0.0f, -0.0f}, .tangent{0.0f,  1.0f, 0.0f}, .bitangent{0.0f,  -0.0f, 1.0f},  .textureCoordinates{0.375f, 0.75f}},
			Object::Vertex{.position{1.0f,  -1.0f, 1.0f},  .normal{-0.0f, -0.0f, 1.0f},  .tangent{0.0f,  1.0f, 0.0f}, .bitangent{-1.0f, 0.0f,  0.0f},  .textureCoordinates{0.375f, 0.75f}},
			Object::Vertex{.position{1.0f,  1.0f,  1.0f},  .normal{1.0f,  -0.0f, -0.0f}, .tangent{0.0f,  1.0f, 0.0f}, .bitangent{0.0f,  -0.0f, 1.0f},  .textureCoordinates{0.625f, 0.75f}},
			Object::Vertex{.position{1.0f,  1.0f,  1.0f},  .normal{-0.0f, -0.0f, 1.0f},  .tangent{0.0f,  1.0f, 0.0f}, .bitangent{-1.0f, 0.0f,  0.0f},  .textureCoordinates{0.625f, 0.75f}},
		};
		constexpr std::array CUBE_INDICES{
			Object::Index{12}, Object::Index{8},  Object::Index{10}, Object::Index{15}, Object::Index{16}, Object::Index{9},
			Object::Index{19}, Object::Index{20}, Object::Index{17}, Object::Index{23}, Object::Index{11}, Object::Index{21},
			Object::Index{6},  Object::Index{0},  Object::Index{2},  Object::Index{3},  Object::Index{5},  Object::Index{7},
			Object::Index{12}, Object::Index{14}, Object::Index{8},  Object::Index{15}, Object::Index{18}, Object::Index{16},
			Object::Index{19}, Object::Index{22}, Object::Index{20}, Object::Index{23}, Object::Index{13}, Object::Index{11},
			Object::Index{6},  Object::Index{4},  Object::Index{0},  Object::Index{3},  Object::Index{1},  Object::Index{5},
		};
		// clang-format on

		std::vector<Object> quadObjects{};
		quadObjects.push_back(Object{
			.mesh{Object::VERTICES_USAGE, Object::INDICES_USAGE, Object::INSTANCES_USAGE, QUAD_VERTICES, QUAD_INDICES, {}},
			.material{
				.diffuseMap{},
				.specularMap{},
				.normalMap{},
				.emissiveMap{},
				.diffuseColor{1.0f, 1.0f, 1.0f},
				.specularColor{1.0f, 1.0f, 1.0f},
				.normalScale{1.0f, 1.0f, 1.0f},
				.emissiveColor{0.0f, 0.0f, 0.0f},
				.specularExponent = 1.0f,
				.dissolveFactor = 0.0f,
				.occlusionFactor = 1.0f,
			},
			.indexCount = QUAD_INDICES.size(),
		});

		std::vector<Object> cubeObjects{};
		cubeObjects.push_back(Object{
			.mesh{Object::VERTICES_USAGE, Object::INDICES_USAGE, Object::INSTANCES_USAGE, CUBE_VERTICES, CUBE_INDICES, {}},
			.material{
				.diffuseMap{},
				.specularMap{},
				.normalMap{},
				.emissiveMap{},
				.diffuseColor{1.0f, 1.0f, 1.0f},
				.specularColor{1.0f, 1.0f, 1.0f},
				.normalScale{1.0f, 1.0f, 1.0f},
				.emissiveColor{0.0f, 0.0f, 0.0f},
				.specularExponent = 1.0f,
				.dissolveFactor = 0.0f,
				.occlusionFactor = 1.0f,
			},
			.indexCount = CUBE_INDICES.size(),
		});

		std::construct_at(QUAD, std::move(quadObjects));
		try {
			std::construct_at(CUBE, std::move(cubeObjects));
		} catch (...) {
			std::destroy_at(QUAD);
			throw;
		}
	}
	++sharedModelReferenceCount;
}

void Model::destroySharedModels() noexcept {
	if (sharedModelReferenceCount-- == 1) {
		std::destroy_at(CUBE);
		std::destroy_at(QUAD);
	}
}

} // namespace donut::graphics
