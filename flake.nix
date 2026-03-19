{
  description = "dev shell";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";

  outputs = { self, nixpkgs }:
  let
    system = "x86_64-linux";
    pkgs = import nixpkgs { inherit system; };
  in {
    devShells.${system} = {
      default = pkgs.mkShell {
        buildInputs = [
          pkgs.gcc13
          pkgs.clang-tools
          pkgs.pkg-config
          pkgs.flint
          pkgs.vscodium
        ];
      };
    };
  };
}