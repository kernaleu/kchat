{
  inputs.nixpkgs.url = "github:NixOs/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }: let
    lib = nixpkgs.lib;

    forSystem = f: system: f {
      inherit system;
      pkgs = import nixpkgs { inherit system; };
    };

    supportedSystems = lib.platforms.unix;
    forAllSystems = f: lib.genAttrs supportedSystems (forSystem f);
  in {
    devShells = forAllSystems ({ system, pkgs }: {
      default = pkgs.mkShell {
        name = "dev";

        buildInputs = with pkgs; [
          gdb
          ccls
          clang-tools
          libxcrypt
        ];

	shellHook = ''
          export PS1="\033[0;32m[kchat]\033[0m $PS1"
        '';
      };
    });
  };
}
