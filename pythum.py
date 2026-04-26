from math import sqrt, log2
import random
from time import time_ns

"""
    TODO state tracing to get smaller qbit set states from larger ones
"""

def display_state_in_ket(state: list[complex], error: float = 0, mode: str = 'txt') -> str:
    def binary_str(num: int) -> str:
        o: str = ''
        if num == 0:
            return '0'
        if num < 0:
            o += '-'
            num *= -1
        mask: int = 1 << int(log2(num))
        while mask > 0:
            o += '1' if num & mask else '0'
            mask = int(mask >> 1)
        return o
    match mode:
        case 'latex' | 'Latex':
            output = r"$"
            for i in range(len(state)):
                if state[i].conjugate() * state[i] <= error * error:
                    continue
                if len(output):
                    output += " + "
                ISTR = binary_str(i)
                PAD  = '' + (int(log2(len(state))) - len(ISTR)) * '0'
                output += fr"({state[i]})\left|{PAD}{ISTR}\right>"
            output += r"$"
        case _:
            output = ""
            for i in range(len(state)):
                if abs(state[i]) <= error:
                    continue
                if len(output):
                    output += " + "
                ISTR = binary_str(i)
                PAD  = '' + (int(log2(len(state))) - len(ISTR)) * '0'
                output += f"({state[i]})|{PAD}{ISTR}>"
    return output

def tensor_product(state1: list[list[complex]], state2: list[list[complex]]) -> list[list[complex]]:
    state: list[list[complex]] = []
    for i1 in range(len(state1)):
        for i2 in range(len(state2)):
            state.append([])
            for j1 in range(len(state1[0])):
                for j2 in range(len(state2[0])):
                    state[i1 * len(state2) + i2].append(state1[i1][j1] * state2[i2][j2])
    return state        


class Gate:
    def __init__(self, gate: list[list[complex]] | str, targets: list[int], label: str | None=None, control: list[int] | None=None, ctrl_value: list[bool] | None=None):
        self.label      = gate if type(gate) == str and label == None else label
        self.gate       = gate
        self.targets    = targets
        self.control    = control
        self.ctrl_value = [True for _ in range(len(self.control))] if self.control != None and ctrl_value == None else ctrl_value

    def __str__(self) -> str:
        NAME = self.label if self.label != None else "GATE"
        TMIN = min(self.targets) if self.targets != None else 0
        TMAX = max(self.targets) if self.targets != None else 0
        CMIN = min(self.control) if self.control != None else 0
        CMAX = max(self.control) if self.control != None else 0
        NLEN = len(NAME) + 2
        NLEN_T = 0 if self.targets == None else len(self.targets)
        NLEN_T = max(NLEN - len(str(NLEN_T)), 0)
        def name_block() -> str:
            s = ""
            #if ctrl_top:
            #    s += '/' + '-' * int((NLEN - 1) / 2) + '|' + '-' * int((NLEN - 1) / 2) + '\\\n'
            #else:
            #    s += '/' + '-' * NLEN + '\\\n'
            begin = TMIN
            end   = TMAX
            p = begin
            i = 0
            if len(self.targets) > 1:
                while p < begin + (end - begin) / 2 - 1:
                    if len(self.targets) > 1 and p in self.targets:
                        dummy = str(self.targets.index(p))
                        s += '|' + dummy + ' ' * min(NLEN_T, max(0, NLEN - len(dummy))) + '|\n'
                        i += 1
                    else:
                        s += '|' + ' ' * NLEN + '|\n'
                    s += '|' + ' ' * NLEN + '|\n'
                    p+=1
                if p < begin + (end - begin) / 2:
                    if len(self.targets) > 1 and p in self.targets:
                        dummy = str(self.targets.index(p))
                        s += '|' + dummy + ' ' * min(NLEN_T, max(0, NLEN - len(dummy))) + '|\n'
                        i += 1
                    else:
                        s += '|' + ' ' * NLEN + '|\n'
                    p+=1
            s += '| ' + NAME + ' |\n'
            if len(self.targets) > 1:
                while p < end:
                    if len(self.targets) > 1 and p in self.targets:
                        dummy = str(self.targets.index(p))
                        s += '|' + dummy + ' ' * min(NLEN_T, max(0, NLEN - len(dummy))) + '|\n'
                        i += 1
                    else:
                        s += '|' + ' ' * NLEN + '|\n'
                    s += '|' + ' ' * NLEN + '|\n'
                    p+=1
                if len(self.targets) > 1 and p <= end and self.targets != None:
                    if p in self.targets:
                        dummy = str(self.targets.index(p))
                        s += '|' + dummy + ' ' * min(NLEN_T, max(0, NLEN - len(dummy))) + '|\n'
                        i += 1
                    else:
                        s += '|' + ' ' * NLEN + '|\n'
                    p+=1
            #if ctrl_bottom:
            #    s += '\\' + '-' * int((NLEN - 1) / 2) + '|' + '-' * int((NLEN - 1) / 2) + '/'
            #else:
            #    s += '\\' + '-' * NLEN + '/\n'
            return s
        
        string = '/' + '-' * NLEN + '\\\n'
        MAX_WIDTH = max(NLEN, len(str(max(self.targets))) if self.targets != None else 0)
        XPAD0 = int(MAX_WIDTH / 2)
        XPAD1 = int((MAX_WIDTH - 1) / 2)
        p = 0
        for i in range(CMIN, TMIN * bool(self.control != None)):
            #string += XPAD0 * ' ' + '|' + XPAD1 * ' ' + '\n'
            if i in self.control:
                symbol = '@' if self.ctrl_value[p] else 'O'
                p+=1
            else:
                symbol = '|'
            string += '|' + XPAD0 * ' ' + symbol + XPAD1 * ' ' + '|\n'
            string += '|' + XPAD0 * ' ' + '|' + XPAD1 * ' ' + '|\n'
        
        string += name_block()

        for i in range(TMAX + 1, (CMAX + 1) * bool(self.control != None)):
            string += '|' + XPAD0 * ' ' + '|' + XPAD1 * ' ' + '|\n'
            if i in self.control:
                symbol = '@' if self.ctrl_value[p] else 'O'
                p+=1
            else:
                symbol = '|'
            string += '|' + XPAD0 * ' ' + symbol + XPAD1 * ' ' + '|\n'
            #string += XPAD0 * ' ' + '|' + XPAD1 * ' ' + '\n'

        string += '\\' + '-' * NLEN + '/\n'

        return string

        return self.get_str_repr()

class QuantumCircuitComponent:
    def __init__(self, what: str | Gate, qbit: int, name: str | None = None, cbit: int | None = None):
        self.what       = what
        self.qbit       = qbit
        self.cbit       = cbit
        if name == None:
            self.name   = what if type(what) == str else what.label
        else:
            self.name   = name
        self.lenght     = len(name) if name != None else len("COMP")


class QuantumCircuit:
    """
        QuantumCircuit Base class (NOT OPTIMIZED)

        TODO: optimize and add functionality

        .. code-block:: text
            example simple entanglement:

            import pythum

            qc = pythum.QuantumCircuit(2)
            
            qc.h(0)
            
            qc.cx(0, 1)
            print(qc.draw())
            
            qc.simulate()
            
            print(pythum.display_state_in_ket(qc.qbits_state))

            output:

                  /---\ /---\ 
            q_0 --| h |-| @ |-
                  \---/ | | | 
            q_1 --------| x |-
                        \---/ 
            
            (0.7071067811865475)|00> + (0.7071067811865475)|11>

            example full example:
            import math

            qc = QuantumCircuit(5)

            qc.h(0)

            qc.cx(0, 2)

            qc.measure([0, 1], [0, 1])

            qc.unitary(
                [
                    [1, 0, 0, 0],
                    [0, 0, 1, 0],
                    [0, 1, 0, 0],
                    [0, 0, 0, 1]
                ],
                [2, 3],
                "SWAP"
            )

            RSQRT = 1 / math.sqrt(2)

            qc.append(
                Gate(
                    [
                        [1, 0    , 0     , 0],
                        [0, RSQRT, -RSQRT, 0],
                        [0, RSQRT,  RSQRT, 0],
                        [0, 0    ,  0    , 1]
                    ],
                    [2, 4],
                    "BS",
                    [0, 1],
                    [1, 0]
                ),
                [0, 1, 4, 2]
            )

            qc.controled_trivial(
                'y',
                [0, 1, 3],
                2,
                [0, 1, 1]
            )

            qc.measure([3, 4])

            print(qc.draw())


            # simulation

            initial_state = qc.qbits_state.copy()

            counts: list[int] = [0 for _ in range(qc.number_of_qbits)]

            SHOTS = 1000

            for _ in range(SHOTS):
                qc.simulate(initial_state)
                for i in range(len(counts)):
                    counts[i] += qc.cbits & (1 << i)

            # this will fluctuate due to probabilistic behaviour
            print(counts)

            output:
                  /---\ /---\  /--------\                      /----\ /---\ 
            q_0 --| h |-| @ |--| m -> 0 |----------------------|  @ |-| O |-
                  \---/ | | |  \--------/  /--------\          |  | | | | | 
            q_1 --------| | |--------------| m -> 1 |----------|  O |-| @ |-
                        | | |              \--------/ /------\ |  | | | | | 
            q_2 --------| x |-------------------------|0     |-|1   |-| y |-
                        \---/                         | SWAP | | BS | | | |  /--------\ 
            q_3 --------------------------------------|1     |-|    |-| @ |--| m -> 3 |-
                                                      \------/ |    | \---/  \--------/  /--------\ 
            q_4 -----------------------------------------------|0   |--------------------| m -> 4 |-
                                                               \----/                    \--------/ 

            [492, 0, 0, 3936, 0]

    """
    def __init__(self, number_of_qbits: int):
        MAX_NUMBER_OF_QBITS = 15
        if number_of_qbits > MAX_NUMBER_OF_QBITS:
            raise MemoryError(f"maximum allowed number of qbits({MAX_NUMBER_OF_QBITS}) exceeded ({number_of_qbits})")
        self.number_of_qbits = number_of_qbits
        self.qbits_state: list[complex] = [0 for _ in range(2 ** number_of_qbits)]
        self.qbits_state[0] = 1
        self.circuit: list[QuantumCircuitComponent] = []
        self.cbits: int = 0

    def h(self, qbit: int):
        self.circuit.append(QuantumCircuitComponent('h', qbit=qbit))
    def x(self, qbit: int):
        self.circuit.append(QuantumCircuitComponent('x', qbit=qbit))
    def y(self, qbit: int):
        self.circuit.append(QuantumCircuitComponent('y', qbit=qbit))
    def z(self, qbit: int):
        self.circuit.append(QuantumCircuitComponent('z', qbit=qbit))
    def measure(self, qbit: int | list[int], cbit: int | list[int] | None = None):
        """
        add a measurement operation to circuit
        Args:
            qbit:
                the index or sequence of qbits to measure
            cbit:
                the index or sequence of qbits to write the measurement, if None the same sequence as qbit will be taken,
                the terms not provided in this sequence will be set to the corresponding index in qbit
        """
        if type(qbit) == int:
            if cbit == None:
                cbit = qbit
            self.circuit.append(QuantumCircuitComponent('m', qbit=qbit, cbit=cbit))
        else:
            if cbit == None:
                cbit = qbit
            if type(cbit) == int:
                for i in range(len(qbit)):
                    self.circuit.append(QuantumCircuitComponent('m', qbit=qbit[i], cbit=cbit))
            else:
                if len(cbit) < len(qbit):
                    cbit = cbit + qbit[len(cbit) : ]
                for i in range(len(qbit)):
                    self.circuit.append(QuantumCircuitComponent('m', qbit=qbit[i], cbit=cbit[i]))
    def reset(self, qbit: int | list[int] | None = None, value: bool = 0):
        """
        reset one, a sequence of, or all qbits
        Args:
            qbits: the qbit(s) to be reset or None to reset all qbits
            value: the value to collapse the qbit to (0 or 1)
        """
        if qbit == None:
            for q in range(self.number_of_qbits):
                self.circuit.append(QuantumCircuitComponent('r', qbit=q, cbit=value))
        elif type(qbit) == int:
            self.circuit.append(QuantumCircuitComponent('r', qbit=qbit, cbit=value))
        else:
            for q in qbit:
                self.circuit.append(QuantumCircuitComponent('r', qbit=q, cbit=value))

    def controled_trivial(self, gate_specifier: str, control: list[int], target: int, control_value: list[bool]):
        """
        gate_specifiers are:
            'x' -> pauli x gate
            'y' -> pauli y gate
            'z' -> pauli z gate
            'h' -> pauli h gate
        """

        GATE = Gate(
            gate        = gate_specifier,
            targets     = [target],
            label       = gate_specifier,
            control     = control,
            ctrl_value  = control_value
        )
        self.circuit.append(
            QuantumCircuitComponent(
                what = GATE,
                qbit = min(min(control), target),
            )
        )
    def ch(self, control: int, target: int, control_value: bool = 1):
        return self.controled_trivial('h', [control], target, [control_value])
    def cx(self, control: int, target: int, control_value: bool = 1):
        return self.controled_trivial('x', [control], target, [control_value])
    def cy(self, control: int, target: int, control_value: bool = 1):
        return self.controled_trivial('y', [control], target, [control_value])
    def cz(self, control: int, target: int, control_value: bool = 1):
        return self.controled_trivial('z', [control], target, [control_value])
    def cch(self, control1: int, control2: int, target: int, control_value: int = 3):
        return self.controled_trivial('h', [control1, control2], target, [control_value & 2, control_value & 1])
    def ccx(self, control1: int, control2: int, target: int, control_value: int = 3):
        return self.controled_trivial('x', [control1, control2], target, [control_value & 2, control_value & 1])
    def ccy(self, control1: int, control2: int, target: int, control_value: int = 3):
        return self.controled_trivial('y', [control1, control2], target, [control_value & 2, control_value & 1])
    def ccz(self, control1: int, control2: int, target: int, control_value: int = 3):
        return self.controled_trivial('z', [control1, control2], target, [control_value & 2, control_value & 1])
    
    def unitary(self, obj: list[list], qbits: list[int], label: str | None = None):
        self.circuit.append(
            QuantumCircuitComponent(
                what = Gate(obj, qbits, label),
                qbit = min(qbits),
                name = label
            )
        )
    def append(self, instruction: Gate, qargs: list[int] | None = None):
        if type(instruction) == QuantumCircuit:
            raise RuntimeError("TODO type(instruction) == QuantumCircuit")
        if instruction.control != None:
            instruction.control = qargs[ : len(instruction.control)]
        instruction.targets = qargs[len(instruction.control) if instruction.control != None else 0 : ]
        self.circuit.append(
            QuantumCircuitComponent(
                what = instruction,
                qbit = min(qargs)
            )
        )

    def draw(self) -> str:
        output: list[str] = ["" for _ in range(2 * self.number_of_qbits + 1)]
        for i in range(1, len(output), 2):
            output[i] += f'q_{int(i / 2)} -'
        def align(begin: int = 0, end: int = len(output)):
            pos = 0
            for i in range(begin, end):
                pos = max(pos, len(output[i]))
            for i in range(begin, end):
                output[i] += ('-' if i % 2 else ' ') * (pos - len(output[i]))

        for component in self.circuit:
            if type(component.what) == str:
                align(2 * component.qbit, 2 * component.qbit + 2 + 1)
                if component.what == 'm' or component.what == 'r':
                    label: str = component.what + ' -> ' + str(component.cbit)
                    output[2 * component.qbit + 0] += ' /-' + '-' * len(label) + '-\\ '
                    output[2 * component.qbit + 1] += '-| ' + label + ' |-'
                    output[2 * component.qbit + 2] += ' \\-' + '-' * len(label) + '-/ '
                else:
                    output[2 * component.qbit + 0] += ' /-' + '-' * len(component.what) + '-\\ '
                    output[2 * component.qbit + 1] += '-| ' + component.what + ' |-'
                    output[2 * component.qbit + 2] += ' \\-' + '-' * len(component.what) + '-/ '
            else:
                gate_str  = str(component.what)
                gate_repr = gate_str.split('\n')
                START     = min(len(output), 2 * component.qbit)
                END       = min(len(output), 2 * component.qbit + len(gate_repr))
                align(START, END)
                for i in range(START, END):
                    output[i] += gate_repr[i - START] + ('-' if i % 2 else ' ')
        
        string: str = ""
        for line in output:
            string += line + '\n'
        return string
    
    def perform_x(self, qbit: int):
        for i in range(len(self.qbits_state)):
            if(0 == ((i >> qbit) & 1)):
                j = i | (2 ** qbit)
                tmp: complex = self.qbits_state[i]
                self.qbits_state[i] = self.qbits_state[j]
                self.qbits_state[j] = tmp
    def perform_y(self, qbit: int):
        for i in range(len(self.qbits_state)):
            if(0 == ((i >> qbit) & 1)):
                j = i | (2 ** qbit)
                tmp: complex = self.qbits_state[i]
                self.qbits_state[i] = -1j * self.qbits_state[j]
                self.qbits_state[j] = tmp
    def perform_z(self, qbit: int):
        for i in range(len(self.qbits_state)):
            if(0 == ((i >> qbit) & 1)):
                j = i | (2 ** qbit)
                self.qbits_state[j] *= -1
    def perform_h(self, qbit: int):
        RSQRT2 = 1 / sqrt(2)
        for i in range(len(self.qbits_state)):
            if(0 == ((i >> qbit) & 1)):
                j = i | (2 ** qbit)
                a: complex = self.qbits_state[i]
                b: complex = self.qbits_state[j]
                self.qbits_state[i] = (a + b) * RSQRT2
                self.qbits_state[j] = (a - b) * RSQRT2
    def perform_measuremnt(self, qbit: int, cbit: int) -> bool:
        # compute probabilities
        p0 = 0.0
        for i in range(len(self.qbits_state)):
            if ((i >> qbit) & 1) == 0:
                p0 += abs(self.qbits_state[i])**2

        # sample outcome
        random.seed(time_ns())
        r = random.random()
        outcome = 0 if r < p0 else 1

        # collapse + normalize
        norm = sqrt(p0 if outcome == 0 else (1 - p0))

        for i in range(len(self.qbits_state)):
            if ((i >> qbit) & 1) != outcome:
                self.qbits_state[i] = 0j
            else:
                self.qbits_state[i] /= norm
        
        self.cbits &= ~(1 << cbit)
        self.cbits |= outcome << cbit

        return outcome
    def perform_reset(self, qbit: int, outcome: bool = 0) -> float:
        """
        Returns:
            the probability of measuring the given state at the moment
        """
        # compute probabilities
        p0: float = 0.0
        for i in range(len(self.qbits_state)):
            if ((i >> qbit) & 1) == 0:
                p0 += abs(self.qbits_state[i])**2

        # collapse + normalize
        norm = sqrt(p0 if outcome == 0 else (1 - p0))

        for i in range(len(self.qbits_state)):
            if ((i >> qbit) & 1) != outcome:
                self.qbits_state[i] = 0j
            else:
                self.qbits_state[i] /= norm

        return p0

    def perform_trivial(self, identifier: str, qbit: int):
        match identifier:
            case 'x':
                self.perform_x(qbit)
            case 'y':
                self.perform_y(qbit)
            case 'z':
                self.perform_z(qbit)
            case 'h':
                self.perform_h(qbit)
    def perform_single_qbit_gate(self, qbit: int, g00: complex, g01: complex, g10: complex, g11: complex):
        for i in range(len(self.qbits_state)):
            if(0 == ((i >> qbit) & 1)):
                j = i | (2 ** qbit)
                a: complex = self.qbits_state[i]
                b: complex = self.qbits_state[j]
                self.qbits_state[i] = g00 * a + g01 * b
                self.qbits_state[j] = g10 * a + g11 * b

    def test_ctrl(index: int, control: list[int], ctrl_value: list[bool]) -> bool:
        for i in range(len(control)):
            if ((index >> control[i]) & 1) != ctrl_value[i]:
                return False
        return True
            
    def perform_cx(self, control: list[int], target: int, ctrl_value: list[bool] | None = None):
        if ctrl_value == None:
            ctrl_value = [True for _ in range(len(control))]
        for i in range(len(self.qbits_state)):
            if QuantumCircuit.test_ctrl(i, control, ctrl_value) and (((i >> target) & 1) == 0):
                j = i | (1 << target)
                tmp = self.qbits_state[i]
                self.qbits_state[i] = self.qbits_state[j]
                self.qbits_state[j] = tmp
    def perform_cy(self, control: list[int], target: int, ctrl_value: list[bool] | None = None):
        if ctrl_value == None:
            ctrl_value = [True for _ in range(len(control))]
        for i in range(len(self.qbits_state)):
            if QuantumCircuit.test_ctrl(i, control, ctrl_value) and (((i >> target) & 1) == 0):
                j = i | (1 << target)
                tmp = self.qbits_state[i]
                self.qbits_state[i] = -1j * self.qbits_state[j]
                self.qbits_state[j] = tmp
    def perform_cz(self, control: list[int], target: int, ctrl_value: list[bool] | None = None):
        if ctrl_value == None:
            ctrl_value = [True for _ in range(len(control))]
        for i in range(len(self.qbits_state)):
            if QuantumCircuit.test_ctrl(i, control, ctrl_value) and (((i >> target) & 1) == 0):
                j = i | (1 << target)
                self.qbits_state[j] *= -1
    def perform_ch(self, control: list[int], target: int, ctrl_value: list[bool] | None = None):
        if ctrl_value == None:
            ctrl_value = [True for _ in range(len(control))]
        RSQRT2 = 1 / sqrt(2)
        for i in range(len(self.qbits_state)):
            if QuantumCircuit.test_ctrl(i, control, ctrl_value) and (((i >> target) & 1) == 0):
                j = i | (1 << target)
                a = self.qbits_state[i]
                b = self.qbits_state[j]
                self.qbits_state[i] = (a + b) * RSQRT2
                self.qbits_state[j] = (a - b) * RSQRT2
    def perform_controled_trivial(self, control: list[int], identifier: str, qbit: int, ctrl_value: list[bool] | None = None):
        match identifier:
            case 'x':
                self.perform_cx(control, qbit, ctrl_value)
            case 'y':
                self.perform_cy(control, qbit, ctrl_value)
            case 'z':
                self.perform_cz(control, qbit, ctrl_value)
            case 'h':
                self.perform_ch(control, qbit, ctrl_value)
    def perform_single_qbit_controled_gate(self, control: list[int], target: int, g00: complex, g01: complex, g10: complex, g11: complex, ctrl_value: list[bool] | None = None):
        if ctrl_value == None:
            ctrl_value = [True for _ in range(len(control))]
        for i in range(len(self.qbits_state)):
            if QuantumCircuit.test_ctrl(i, control, ctrl_value) and (((i >> target) & 1) == 0):
                j = i | (1 << target)
                a = self.qbits_state[i]
                b = self.qbits_state[j]
                self.qbits_state[i] = g00 * a + g01 * b
                self.qbits_state[j] = g10 * a + g11 * b
    
    def perform_multi_qubit_gate(self, targets: list[int], U: list[list[complex]]):
        k = len(targets)
        dim = 1 << k

        for i in range(self.number_of_qbits):

            # process only if all target bits = 0
            if any((i >> t) & 1 for t in targets):
                continue

            indices = [0] * dim
            vec: list[complex] = [0] * dim

            # build indices and extract amplitudes
            for b in range(dim):
                idx = i
                for t_i, t in enumerate(targets):
                    if (b >> t_i) & 1:
                        idx |= (1 << t)
                    else:
                        idx &= ~(1 << t)
                indices[b] = idx
                vec[b] = self.qbits_state[idx]

            # apply matrix
            out: list[complex] = [0] * dim
            for r in range(dim):
                s: complex = 0
                for c in range(dim):
                    s += U[r][c] * vec[c]
                out[r] = s

            # write back
            for r in range(dim):
                self.qbits_state[indices[r]] = out[r]
    def perform_multi_qubit_controled_gate(self, controls: list[int], ctrl_value: list[bool], targets: list[int], U: list[list[complex]]):
        k = len(targets)
        dim = 1 << k

        for i in range(self.number_of_qbits):

            # process only if all target bits = 0 and all controls test True
            if any((i >> t) & 1 for t in targets) or not QuantumCircuit.test_ctrl(i, controls, ctrl_value):
                continue

            indices = [0] * dim
            vec: list[complex] = [0] * dim

            # build indices and extract amplitudes
            for b in range(dim):
                idx = i
                for t_i, t in enumerate(targets):
                    if (b >> t_i) & 1:
                        idx |= (1 << t)
                    else:
                        idx &= ~(1 << t)
                indices[b] = idx
                vec[b] = self.qbits_state[idx]

            # apply matrix
            out: list[complex] = [0] * dim
            for r in range(dim):
                s: complex = 0
                for c in range(dim):
                    s += U[r][c] * vec[c]
                out[r] = s

            # write back
            for r in range(dim):
                self.qbits_state[indices[r]] = out[r]
    def perform_gate(self, gate: Gate):
        if gate.control == None:
            if type(gate.gate) == str:
                for target in gate.targets:
                    self.perform_trivial(gate.gate, target)
            elif len(gate.targets) == 1:
                self.perform_single_qbit_gate(gate.targets[0], gate.gate[0][0], gate.gate[0][1], gate.gate[1][0], gate.gate[1][1])
            else:
                self.perform_multi_qubit_gate(gate.targets, gate.gate)
        elif type(gate.gate) == str:
            for target in gate.targets:
                self.perform_controled_trivial(gate.control, gate.gate, target, gate.ctrl_value)
        elif len(gate.targets) == 1:
            if type(gate.gate) == str:
                self.perform_controled_trivial(gate.control, gate.gate, target, gate.ctrl_value)
            else:
                self.perform_single_qbit_controled_gate(gate.control, gate.targets[0], gate.gate[0][0], gate.gate[0][1], gate.gate[1][0], gate.gate[1][1])
        else:
            self.perform_multi_qubit_controled_gate(gate.control, gate.control, gate.targets, gate.gate)

    def simulate(self, initial_state: list[complex] | None = None):
        if initial_state != None:
            mod = 0
            for i in initial_state:
                mod += abs(i)
            if mod == 0:
                return
            self.qbits_state = initial_state.copy()
            for i in range(len(initial_state)):
                self.qbits_state[i] /= mod
            for _ in range(len(initial_state), 2 ** self.number_of_qbits):
                self.qbits_state.append(0)
        for component in self.circuit:
            if type(component.what) == str:
                if component.what == 'm':
                    self.perform_measuremnt(component.qbit, component.cbit)
                elif component.what == 'r':
                    self.perform_reset(component.qbit, component.cbit if component.cbit != None else 0)
                else:
                    self.perform_trivial(component.what, component.qbit)
            else:
                self.perform_gate(component.what)

