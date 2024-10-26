files = {
    "waves": 2,
    # "particles_coulomb": 3,
    "stable_fluids": 4,
    "reaction_diffusion": 16,
    "schrod_leapfrog": 9,
    "schrod_splitstep_3d": 13,
    "free_space_position": 18,
    "isf_splitstep": 5,
    "sph": 6,
    "mol_dynamics": 7,
    "mol_dynamics_mt": 8,
    # "schrod_leapfrog_3d": 10,
    "pauli_leapfrog_3d": 11,
    "schrod_splitstep": 12,
    "dirac_leapfrog": 14,
    "dirac_splitstep_3d": 15,
    # "electrodynamics_3d": 17, 
}

template = \
"""<!DOCTYPE html>
<meta http-equiv="Cross-Origin-Embedder-Policy" content="require-corp">
<meta http-equiv="Cross-Origin-Opener-Policy" content="same-origin">
<meta charset="utf-8"/>
<html>
        <!--<h1 style="color:white;">Simulation</h1>>!-->
    <body style="font-family: Arial; background-color: rgb(0, 0, 0);">
        <div style="display: grid; grid-template-columns: 1fr 1fr; text-align: center;">
            <div style="grid-column: 1">
                <h2 style="color: white;">WebAssembly (WASM) build of program</h2>
                <canvas id="canvas", oncontextmenu="event.preventDefault()", width=97%, style="border: solid white 1px; top: 0px; bottom: 0px;">
                </canvas>
            </div>
            <div style="grid-column: 2">
                <h2 style="color: white;">Pertinent source file: {}.cpp</h2>
                <embed type="text/plain" src="./{}.cpp"
                    encoding="utf-8" width="100%" height="75%"
                    style="background-color: white;">
                </embed></br>
                <div style="color: white; offset-inline-start: 5%; text-align: left;">
                Obviously, it would be really nice if you could edit this C++
                source file, dynamically re-compile the WASM binary,
                then update the program displayed on the left.
                Unfortunately,
                I have no idea idea how to do this :(.
                </div>
                <a href=
"./"
                    style="color: lightblue">More WASM programs
                    </a><br/>
                <a href=
"https://github.com/marl0ny/SomeGraphicsStuff/OpenGL/TextureDataObjects"
                    style="color: lightblue">Full source code</a>
            </div>
        </div>
    </body>
    <script type='text/javascript'>
        var canvas = document.getElementById('canvas');
        var Module = {} // Put starting bracket here
          canvas: canvas,
          arguments: ["{}"]
        {}; // Place ending bracket
    </script>
    <!--<script scr="/stats/stats.min.js"></script>!-->
    <!--<script type="text/javascript">
	    var stats = new Stats
    </script>!-->
    <script src="main.js"></script>
</html>
"""

index_template = """
<!DOCTYPE html>
<meta charset="utf-8"/>
<html>
    <body>
        <h1>WASM programs</h1>
        <p>Sorry for the sparse documentation; I have yet to write descriptions on what each of these
        programs actually do here and in their respective pages. For the time being each page will show the program
        itself in the left pane, with its source file on the right.
        Hopefully you can gain the necessary context and information from looking at the
        program's source file itself!
        </p>
        <p>Note that these programs will not work on handheld/mobile devices.
        They may not work properly on all web browsers as well;
        Firefox should be the most compatible browser, since it was used for testing.
        </p>
        <p>
        <h3>Full list of simulations:</h3>
        {}
        </p>
    </body>
</html>"""

for fname in files.keys():
    print(fname)
    with open(f'{fname}.html', 'w') as f:
        # print(files[fname])
        f.write(template.format(fname, fname, '{', files[fname], '}'))

with open('index.html', 'w') as f:
    a_tags = ''.join(
        [
            f'<li style="list-style-type: square;"><a href="./{fname}.html">{fname}</a></li>'
            for fname in files.keys()
        ]
    )
    f.write(index_template.format(a_tags))


