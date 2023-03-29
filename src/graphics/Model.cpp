#include <donut/InputFileStream.hpp>
#include <donut/graphics/Error.hpp>
#include <donut/graphics/Image.hpp>
#include <donut/graphics/Mesh.hpp>
#include <donut/graphics/Model.hpp>
#include <donut/graphics/Texture.hpp>
#include <donut/obj.hpp>

#include <algorithm>        // std::find_if
#include <cstddef>          // std::size_t
#include <exception>        // std::exception
#include <fmt/format.h>     // fmt::format
#include <functional>       // std::hash
#include <glm/glm.hpp>      // glm::...
#include <glm/gtx/norm.hpp> // glm::length2
#include <numbers>          // std::numbers_pi_v
#include <span>             // std::span
#include <string>           // std::string
#include <unordered_map>    // std::unordered_map
#include <utility>          // std::move
#include <vector>           // std::vector

namespace donut {
namespace graphics {

namespace {

void generateNormals(std::span<Model::Object::Vertex> vertices, std::span<const Model::Object::Index> indices) {
	for (Model::Object::Vertex& vertex : vertices) {
		vertex.normal = {0.0f, 0.0f, 0.0f};
	}

	for (std::size_t triangleIndex = 0; triangleIndex + 3 <= indices.size(); triangleIndex += 3) {
		const std::size_t indexA = static_cast<std::size_t>(indices[triangleIndex]);
		const std::size_t indexB = static_cast<std::size_t>(indices[triangleIndex + 1]);
		const std::size_t indexC = static_cast<std::size_t>(indices[triangleIndex + 2]);

		const glm::vec3 ab = vertices[indexB].position - vertices[indexA].position;
		const glm::vec3 ac = vertices[indexC].position - vertices[indexA].position;
		const glm::vec3 bc = vertices[indexC].position - vertices[indexB].position;

		const glm::vec3 normal = glm::cross(ab, ac);

		const float squareLengthAB = glm::length2(ab);
		const float squareLengthAC = glm::length2(ac);
		const float squareLengthBC = glm::length2(bc);

		const float lengthAB = (squareLengthAB >= 1e-6f) ? glm::sqrt(squareLengthAB) : 1e-3f;
		const float lengthAC = (squareLengthAC >= 1e-6f) ? glm::sqrt(squareLengthAC) : 1e-3f;
		const float lengthBC = (squareLengthBC >= 1e-6f) ? glm::sqrt(squareLengthBC) : 1e-3f;

		const float influenceAB = glm::dot(ab, ac) / (lengthAB * lengthAC);
		const float influenceAC = -glm::dot(ab, bc) / (lengthAB * lengthBC);
		const float influenceBC = glm::dot(ac, bc) / (lengthAC * lengthBC);

		const float angleAB = (influenceAB >= 1.0f) ? 0.0f : (influenceAB <= -1.0f) ? std::numbers::pi_v<float> : glm::acos(influenceAB);
		const float angleAC = (influenceAC >= 1.0f) ? 0.0f : (influenceAC <= -1.0f) ? std::numbers::pi_v<float> : glm::acos(influenceAC);
		const float angleBC = (influenceBC >= 1.0f) ? 0.0f : (influenceBC <= -1.0f) ? std::numbers::pi_v<float> : glm::acos(influenceBC);

		vertices[indexA].normal += normal * angleAB;
		vertices[indexB].normal += normal * angleAC;
		vertices[indexC].normal += normal * angleBC;
	}

	for (Model::Object::Vertex& vertex : vertices) {
		vertex.normal = glm::normalize(vertex.normal);
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

		const glm::vec3 ab = vertices[indexB].position - vertices[indexA].position;
		const glm::vec3 ac = vertices[indexC].position - vertices[indexA].position;

		glm::vec2 uvAB = vertices[indexB].textureCoordinates - vertices[indexA].textureCoordinates;
		glm::vec2 uvAC = vertices[indexC].textureCoordinates - vertices[indexA].textureCoordinates;

		if (uvAB == glm::vec2{0.0f, 0.0f} && uvAC == glm::vec2{0.0f, 0.0f}) {
			uvAB = ab;
			uvAC = ac;
		}

		const float r = 1.0f / (uvAB.x * uvAC.y - uvAB.y * uvAC.x);
		const glm::vec3 tangent = r * (ab * uvAC.y - ac * uvAB.y);

		vertices[indexA].tangent += tangent;
		vertices[indexB].tangent += tangent;
		vertices[indexC].tangent += tangent;
	}

	for (Model::Object::Vertex& vertex : vertices) {
		vertex.tangent = glm::normalize(vertex.tangent - glm::dot(vertex.tangent, vertex.normal) * vertex.normal);
		vertex.bitangent = glm::cross(vertex.normal, vertex.tangent);
	}
}

[[nodiscard]] Texture loadTexture(const std::string& filepath) {
	return Texture{Image{filepath.c_str(), {.highDynamicRange = filepath.ends_with(".hdr")}}};
}

void loadObjScene(Model& output, const obj::Scene& scene) {
	std::vector<obj::mtl::Library> materialLibraries{};
	materialLibraries.reserve(scene.materialLibraryFilenames.size());
	for (const std::string& materialLibraryFilename : scene.materialLibraryFilenames) {
		materialLibraries.push_back(obj::mtl::Library::parse(InputFileStream::open(materialLibraryFilename.c_str()).readAllIntoString()));
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
									.position = (faceVertex.vertexIndex < scene.vertices.size()) ? scene.vertices[faceVertex.vertexIndex] : glm::vec3{0.0f, 0.0f, 0.0f},
									.normal = (faceVertex.normalIndex < scene.normals.size()) ? scene.normals[faceVertex.normalIndex] : glm::vec3{0.0f, 0.0f, 0.0f},
									.tangent{},
									.bitangent{},
									.textureCoordinates = (faceVertex.textureCoordinateIndex < scene.textureCoordinates.size())
								                              ? scene.textureCoordinates[faceVertex.textureCoordinateIndex]
								                              : glm::vec2{0.0f, 0.0f},
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

			Model::Object::Material groupMaterial{.diffuseMap{}, .specularMap{}, .normalMap{}, .specularExponent = 0.0f};
			if (!group.materialName.empty()) {
				for (const obj::mtl::Library& materialLibrary : materialLibraries) {
					if (const auto it = std::find_if(materialLibrary.materials.begin(), materialLibrary.materials.end(),
							[&](const obj::mtl::Material& material) -> bool { return material.name == group.materialName; });
						it != materialLibrary.materials.end()) {
						const obj::mtl::Material& material = *it;
						if (!material.diffuseMapName.empty()) {
							groupMaterial.diffuseMap = loadTexture(material.diffuseMapName);
						}
						if (!material.specularMapName.empty()) {
							groupMaterial.specularMap = loadTexture(material.specularMapName);
						}
						if (!material.bumpMapName.empty()) {
							groupMaterial.normalMap = loadTexture(material.bumpMapName);
						}
						groupMaterial.specularExponent = material.specularExponent;
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

} // namespace

Model::Model(const char* filepath) {
	try {
		loadObjScene(*this, obj::Scene::parse(InputFileStream::open(filepath).readAllIntoString()));
	} catch (const obj::Error& e) {
		throw Error{fmt::format("Failed to load model \"{}\": Line {}: {}", filepath, e.lineNumber, e.what())};
	} catch (const std::exception& e) {
		throw Error{fmt::format("Failed to load model \"{}\": {}", filepath, e.what())};
	} catch (...) {
		throw Error{fmt::format("Failed to load model \"{}\".", filepath)};
	}
}

} // namespace graphics
} // namespace donut
