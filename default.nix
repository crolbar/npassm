{ lib
, stdenv
, pkg-config
, gnumake
, cmake
, gcc
, openssl
, ncurses
}:
let
    fs = lib.fileset;
    buildInputs = [
        pkg-config
        gnumake
        cmake
        gcc
        openssl
        ncurses
    ];
in
stdenv.mkDerivation {
    pname = "npassm";
    version = "0.1";
    src = fs.toSource {
        root = ./.;
        fileset = fs.difference ./. (fs.maybeMissing ./build);
    };
    inherit buildInputs;

    installPhase = ''
        mkdir -p $out/bin
        cp npassm $out/bin
    '';
}
