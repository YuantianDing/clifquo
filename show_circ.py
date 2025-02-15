
from dataclasses import dataclass
import json

class QuantikzTable:
    def __init__(self, nqubits: int):
        self.rows = [[""] for i in range(nqubits)]
    
    def ensure_row(self, row: int, to: int):
        self.rows[row] += [""] * (to + 1 - len(self.rows[row]))
    def add_col_line(self, a: int, b: int):
        r = range(min(a, b), max(a, b) + 1)
        length = max(len(self.rows[i]) for i in r)
        for i in r:
            self.ensure_row(i, length)
            
    def add(self, circ: list):
        match circ:
            case ["II-CX", q1, q2]:
                self.add_col_line(q1, q2)
                self.rows[q1][-1] = r"\ctrl{" + str(q2 - q1) + r"}"
                self.rows[q2][-1] = r"\targ{}"
            case ["T", q1]:
                self.rows[q1].append(r"\gate{T}")
            case ["="]:
                self.add_col_line(0, len(self.rows) - 1)
                self.rows[0][-1] = r"\midstick[" + str(len(self.rows)) + r",brackets=none]{\text{=}}"
    
    def into_quantikz(self):
        return r"\begin{quantikz}" + "\n" + "\n".join(
            " & ".join(row) + r"\\" for row in self.rows
        ) + "\n" + r"\end{quantikz}" + "\n"
        


print(r"""
\documentclass[tikz]{article}
\usepackage{graphicx}
\usepackage{amsmath} 
\usepackage{tikz}
\usepackage{quantikz}
\usepackage{braket}
\begin{document}
""")


with open('equivalences.json', 'r') as file:
    eqs = json.load(file)
    for eq in eqs:
        table = QuantikzTable(4)
        for g in eq['old']:
            table.add(g)
        table.add(["="])
        for g in eq['new']:
            table.add(g)
        table.add_col_line(0, 3)
        print(table.into_quantikz())

print(r"\end{document}")