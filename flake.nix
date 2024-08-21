{
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = {nixpkgs, ...}: let
    systems = ["x86_64-linux" "aarch64-linux"];
    eachSystem = nixpkgs.lib.genAttrs systems;

    pkgsFor = eachSystem (
      system:
        import nixpkgs {inherit system;}
    );

    mkBulidInputs = pkgs:
      with pkgs; [
        clang-tools
        pkg-config
        gnumake
        cmake
        gcc
        openssl
        ncurses
      ];
  in {
    devShells = eachSystem (
      system: let
        pkgs = pkgsFor.${system};
        nativeBuildInputs = mkBulidInputs pkgs;
      in
        with pkgs; {
          default = mkShell {
            packages = nativeBuildInputs;
          };
        }
    );

    packages = eachSystem (
      system: let
        pkgs = pkgsFor.${system};
        nativeBuildInputs = mkBulidInputs pkgs;

        fs = nixpkgs.lib.fileset;

        npassm = pkgs.stdenv.mkDerivation {
          pname = "npassm";
          version = "0.1";
          src = fs.toSource {
            root = ./.;
            fileset = fs.difference ./. (fs.maybeMissing ./build);
          };

          inherit nativeBuildInputs;
        };
      in {
        inherit npassm;
        default = npassm;
      }
    );
  };
}
