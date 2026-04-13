from math import sqrt


class Gate:
    def __init__(self, gate: list[list[complex]] | str, label: str=None, upchar = None, downchar = None, control: list[int] = None):
        self.label      = gate if type(gate) == str and label == None else label
        self.gate       = gate
        self.control    = control
        self.upchar     = upchar
        self.downchar   = downchar



class QuantumCircuit:
    def __init__(self, number_of_qbits: int, number_of_cbits: int):
        MAX_NUMBER_OF_QBITS = 15
        if number_of_qbits > MAX_NUMBER_OF_QBITS:
            raise MemoryError(f"maximum allowed number of qbits({MAX_NUMBER_OF_QBITS}) exceeded ({number_of_qbits})")
        self.number_of_qbits = number_of_qbits
        self.qbits_state: list[complex] = [0 for _ in range(2 ** number_of_qbits)]
        self.qbits_state[0] = 1
        self.circuit: list[list[Gate]] = [
            [] for _ in range(self.number_of_qbits)
        ]
        self.cbits = [0 for _ in range(number_of_cbits)]

    def single_qbit_gate(self, qbit: int | list[int], gate: Gate):
        if type(qbit) == int:
            self.circuit[qbit].append(gate)
        else:
            raise NotImplementedError(f"TODO: {self.single_qbit_gate} Not implemented yet")
    
    def h(self, qbit: int):
        self.circuit[qbit].append(Gate('h'))
    def x(self, qbit: int):
        self.circuit[qbit].append(Gate('x'))
    def y(self, qbit: int):
        self.circuit[qbit].append(Gate('y'))
    def z(self, qbit: int):
        self.circuit[qbit].append(Gate('z'))

    def controled_trivial(self, gate_specifier: str, control: int, target: int, control_value: bool = 1):
        if control == target:
            raise Exception("control == target")
        
        for _ in range(len(self.circuit[target]) - len(self.circuit[control])):
            self.circuit[control].append(Gate('-'))
        for _ in range(len(self.circuit[control]) - len(self.circuit[target])):
            self.circuit[target].append(Gate('-'))


        STEP = 1 if target > control else -1
        i = control + STEP
        while i != target:
            for _ in range(len(self.circuit[i]) - len(self.circuit[control])):
                self.circuit[control].append(Gate('-'))
                self.circuit[target ].append(Gate('-'))
            for _ in range(len(self.circuit[control]) - len(self.circuit[i])):
                self.circuit[i].append(Gate('-'))
            self.circuit[i].append(Gate('|'))
            i += STEP

        self.circuit[control].append(
            Gate(
                f"{'@' if control_value == 1 else control_value}",
                upchar  = '|' if STEP==-1 else None,
                downchar= '|' if STEP== 1 else None
            )
        )
        self.circuit[target].append(
            Gate(
                gate_specifier,
                label    = gate_specifier,
                downchar = '|' if STEP==-1 and abs(target - control) > 1 else None,
                upchar   = '|' if STEP== 1 and abs(target - control) > 1 else None,
                control  = control
            )
        )
    def ch(self, control: int, target: int, control_value: bool = 1):
        return self.controled_trivial('h', control, target, control_value)
    def cx(self, control: int, target: int, control_value: bool = 1):
        return self.controled_trivial('x', control, target, control_value)
    def cy(self, control: int, target: int, control_value: bool = 1):
        return self.controled_trivial('y', control, target, control_value)
    def cz(self, control: int, target: int, control_value: bool = 1):
        return self.controled_trivial('z', control, target, control_value)


    def draw(self) -> str:
        def put_in_str(buff: str, input: str, pos: int) -> str:
            if pos < len(buff):
                return buff[ : pos] + input + buff[pos : ]
            for _ in range(pos - len(buff)):
                buff += ' '
            buff += input
            return buff
        
        lines: list[str] = ["" for _ in range(2 * len(self.circuit) + 1)]
        
        MAX_WIRE_LEN = max(len(wire) for wire in self.circuit)

        for qbit in range(len(self.circuit)):
            lines[2 * qbit + 1] += f"q{qbit}: "

        for i in range(MAX_WIRE_LEN):
            max_label_len = 0
            for qbit in range(len(self.circuit)):
                if i >= len(self.circuit[qbit]):
                    continue
                max_label_len = max(max_label_len, len(self.circuit[qbit][i].label))
            for qbit in range(len(self.circuit)):
                if i >= len(self.circuit[qbit]):
                    continue
                LABEL_LEN = len(self.circuit[qbit][i].label)
                preffix_len = int((max_label_len - LABEL_LEN) / 2)
                suffix_len = max_label_len - LABEL_LEN - preffix_len
                if self.circuit[qbit][i].upchar != None:
                    lines[2 * qbit] = put_in_str(
                        lines[2 * qbit],
                        self.circuit[qbit][i].upchar,
                        preffix_len + len(lines[2 * qbit + 1]) + 2 + int(LABEL_LEN / 2)
                    )
                if self.circuit[qbit][i].downchar != None:
                    lines[2 * qbit + 2] = put_in_str(
                        lines[2 * qbit + 2],
                        self.circuit[qbit][i].downchar,
                        preffix_len + len(lines[2 * qbit + 1]) + 2 + int(LABEL_LEN / 2)
                    )
                lines[2 * qbit + 1] += '-' * preffix_len + f"- {self.circuit[qbit][i].label} -" + '-' * suffix_len
        
        output: str = ""
        for l in lines:
            output += l + "\n"
        return output
    
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
    def perform_single_qbit_gate(self, qbit: int, g00: complex, g01: complex, g10: complex, g11: complex):
        for i in range(len(self.qbits_state)):
            if(0 == ((i >> qbit) & 1)):
                j = i | (2 ** qbit)
                a: complex = self.qbits_state[i]
                b: complex = self.qbits_state[j]
                self.qbits_state[i] = g00 * a + g01 * b
                self.qbits_state[j] = g10 * a + g11 * b

    
    def perform_cx(self, control: int, target: int):
        for i in range(len(self.qbits_state)):
            if ((i >> control) & 1) and (((i >> target) & 1) == 0):
                j = i | (1 << target)
                tmp = self.qbits_state[i]
                self.qbits_state[i] = self.qbits_state[j]
                self.qbits_state[j] = tmp
    def perform_cy(self, control: int, target: int):
        for i in range(len(self.qbits_state)):
            if ((i >> control) & 1) and (((i >> target) & 1) == 0):
                j = i | (1 << target)
                tmp = self.qbits_state[i]
                self.qbits_state[i] = -1j * self.qbits_state[j]
                self.qbits_state[j] = tmp
    def perform_cz(self, control: int, target: int):
        for i in range(len(self.qbits_state)):
            if ((i >> control) & 1) and (((i >> target) & 1) == 0):
                j = i | (1 << target)
                self.qbits_state[j] *= -1
    def perform_ch(self, control: int, target: int):
        RSQRT2 = 1 / sqrt(2)
        for i in range(len(self.qbits_state)):
            if ((i >> control) & 1) and (((i >> target) & 1) == 0):
                j = i | (1 << target)
                a = self.qbits_state[i]
                b = self.qbits_state[j]
                self.qbits_state[i] = (a + b) * RSQRT2
                self.qbits_state[j] = (a - b) * RSQRT2
    def perform_single_qbit_controled_gate(self, control: int, target: int, g00: complex, g01: complex, g10: complex, g11: complex):
        for i in range(len(self.qbits_state)):
            if ((i >> control) & 1) and (((i >> target) & 1) == 0):
                j = i | (1 << target)
                a = self.qbits_state[i]
                b = self.qbits_state[j]
                self.qbits_state[i] = g00 * a + g01 * b
                self.qbits_state[j] = g10 * a + g11 * b
    
    def simulate(self, initial_state: list[complex] | None = None):
        if type(initial_state) == list:
            if initial_state != []:
                if len(initial_state) != len(self.qbits_state):
                    raise ValueError(f"initial_state's size ({len(initial_state)}) does not match expected ({len(self.qbits_state)})")
                mod = 0
                for x in initial_state:
                    mod += abs(x) ** 2
                if mod == 0:
                    raise ValueError("Initial state has module zero")
                self.qbits_state = [x / sqrt(mod) for x in initial_state]
        elif initial_state == None:
            self.qbits_state = [0 for _ in range(len(self.qbits_state))]
            self.qbits_state[0] = 1
        else:
            raise TypeError(f"Expected initial_state to be list[complex] or None got {type(initial_state)} instead")
        
        
        MAX_WIRE_LEN = max(len(wire) for wire in self.circuit)
        for i in range(MAX_WIRE_LEN):
            for qbit in range(len(self.circuit)):
                if(i >= len(self.circuit[qbit])):
                    continue
                if type(self.circuit[qbit][i].gate) == str:
                    match self.circuit[qbit][i].gate:
                        case '-' | '|' | '0' | '1':
                            0
                        case 'x':
                            if self.circuit[qbit][i].control != None:
                                self.perform_cx(self.circuit[qbit][i].control, qbit)
                            else:
                                self.perform_x(qbit)
                        case 'y':
                            if self.circuit[qbit][i].control != None:
                                self.perform_cy(self.circuit[qbit][i].control, qbit)
                            else:
                                self.perform_y(qbit)
                        case 'z':
                            if self.circuit[qbit][i].control != None:
                                self.perform_cz(self.circuit[qbit][i].control, qbit)
                            else:
                                self.perform_z(qbit)
                        case 'h':
                            if self.circuit[qbit][i].control != None:
                                self.perform_ch(self.circuit[qbit][i].control, qbit)
                            else:
                                self.perform_h(qbit)
                else:
                    self.perform_single_qbit_gate(qbit, *self.circuit[qbit][i].gate)
