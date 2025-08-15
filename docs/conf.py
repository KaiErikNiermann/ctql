# type: ignore
project = "ctql"
copyright = "2025, Kai Erik Niermann"
author = "Kai Erik Niermann"

extensions = ["breathe", "exhale"]

# If you run doxygen yourself, keep this and point to the XML dir:
breathe_projects = {"ctql": "doxygen/xml"}
breathe_default_project = "ctql"

# Nice defaults for C++
primary_domain = "cpp"
highlight_language = "cpp"
breathe_domain_by_extension = {"hpp": "cpp", "hh": "cpp", "hxx": "cpp", "h": "cpp"}
# Optional: hide long prefixes in indices
cpp_index_common_prefix = ["ctql::"]

templates_path = ["_templates"]
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]

# Exhale config: generate a collapsible API tree
exhale_args = {
    "containmentFolder": "./api",
    "rootFileName": "library_root.rst",
    "rootFileTitle": "ctql API",
    "doxygenStripFromPath": "../",
    "createTreeView": True,
}

# Theme: feel free to keep alabaster; these look cleaner:
html_static_path = ["_static"]
