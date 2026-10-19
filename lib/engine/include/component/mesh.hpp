#pragma once
#include "core/system/asset/assetID.hpp"
#include <string>
namespace comp {
 struct MeshComponent {
  AssetHandle meshID;
 };

 struct MeshRefComponent{
   std::string path;
     int index = 0;
  };
}

