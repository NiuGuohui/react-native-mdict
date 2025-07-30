require "json"

package = JSON.parse(File.read(File.join(__dir__, "package.json")))

Pod::Spec.new do |s|
  s.name         = "Mdict"
  s.version      = package["version"]
  s.summary      = package["description"]
  s.homepage     = package["homepage"]
  s.license      = package["license"]
  s.authors      = package["author"]

  s.platforms    = { :ios => min_ios_version_supported }
  s.source       = { :git => "https://github.com/NiuGuohui/react-native-mdict.git", :tag => "#{s.version}" }

  s.source_files = "cpp/*.{h,cpp,mm}", "ios/**/*.{mm,h}"
#   s.vendored_frameworks = "ios/mdict.xcframework"
  s.vendored_libraries = "mdict/build_ios/libs/libmdict_sim.a"
  s.header_mappings_dir = "ios/mdict.xcframework/ios-arm64/Headers"


  s.pod_target_xcconfig = {
      "CLANG_CXX_LANGUAGE_STANDARD" => "c++20",
      "CLANG_CXX_LIBRARY" => "libc++"
  }

 install_modules_dependencies(s)
end
