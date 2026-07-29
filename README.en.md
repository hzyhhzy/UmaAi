# UmaAI/UmaSimulator · [中文](README.md)

UmaAI is an open-source training simulation and decision-analysis project for *Umamusume: Pretty Derby*. Given a structured representation of the current turn, it evaluates the long-term value of training, rest, outings, races, and scenario-specific actions, then recommends an action aimed at maximizing the final training score.

This project focuses exclusively on score chasing: maximizing the final training score. It does not target training characters for race performance.

Some high-score records in certain Umamusume scenarios were achieved using implementations based on this repository. Because the compliance of such usage is disputed, specific results are not discussed here.

The repository contains a training simulator, a hand-crafted policy, Monte Carlo search, neural-network inference, training-data generation, and model-training code. Its main uses include:

- recommending an action for a particular turn, such as choosing a training option or a scenario upgrade;
- quantitatively studying scenario mechanics and strategy guides, as well as the strength of support-card decks in a scenario;
- learning and experimenting with model-based planning, Monte Carlo search, policy and value networks, search distillation, and approximate policy iteration.

> The UAF scenario is one of the more complete implementations in this repository. If you want to continue development, the `UAF` branch is the recommended reference.

> The author lost interest in Umamusume in early 2026 and stopped playing, so this project is no longer maintained. Interested developers are welcome to continue development using the code in this repository and the explanations in these READMEs. With how capable AI-assisted programming has become, adapting UmaAI to a new scenario should not be especially difficult.

## Input and Output

The state of an ongoing training run—including support-character placement across training facilities, the current turn, current stats, and scenario bonuses—must first be converted into a structured game state. This can be implemented through OCR, manual input, or certain “special methods.” Please research the specific approach yourself; this repository does not provide one.

The UmaAI main program reads an already structured current-turn state. The default `MAINAI` entry point supports three sources:

| `communicationMode` | State source |
|---|---|
| `localfile` | Reads `thisTurn.json` from the program directory |
| `urafile` | Reads `thisTurn.json` from the UmamusumeResponseAnalyzer data directory |
| `websocket` | Receives state through a local websocket client and sends display data back |

After receiving a state, the main program:

1. reconstructs the current `Game` and calculates derived information such as training gains;
2. produces one neural-network or hand-crafted policy recommendation, which is very fast but less accurate;
3. runs root-level Monte Carlo search over all legal actions, which usually takes around ten to twenty seconds and produces a more accurate recommendation;
4. displays the action recommended by UmaAI, candidate values, and the predicted final score in the console;
5. sends display data back to the frontend in websocket mode;
6. waits for the next state update and analyzes it again.

State acquisition and actual action execution are handled by the user or an external frontend. The program itself only analyzes data that has already been obtained; it does not modify or control the original game.

## Algorithm Overview

The core UmaAI algorithm has five parts.

### 1. Training Simulator

The simulator attempts to reproduce Umamusume training mechanics quantitatively, including training-stat formulas, random mechanics, scenario-specific systems and formulas, and score calculation. It can continue from an arbitrary intermediate state to the end of a run, at a speed of up to roughly 5,000 simulated runs per second per CPU thread.

### 2. Hand-Crafted Policy

The hand-crafted policy is a relatively simple manually designed strategy. It considers factors such as stat gains and scenario-mechanic gains, then quickly returns a reasonably good action. It can control later turns during Monte Carlo simulation and generate data for the first neural network.

### 3. Monte Carlo Decision-Making

The search enumerates all legal actions for the current turn. For each candidate, it copies the current state many times, executes that action first, then lets the hand-crafted policy or neural network continue the run inside the simulator and records the distribution of final scores. Each candidate is usually simulated 3,000–10,000 times, reducing Monte Carlo random error to roughly 50 score points.

The action is selected from these final-score distributions, according to either average performance or high-score potential.

### 4. Neural Network (Optional)

For stronger play, a neural network can be trained to replace the original hand-crafted policy.

The network receives a structured game state and outputs a policy and a value. The policy quickly selects later actions during a rollout. The value predicts the mean and random variation of the eventual final score, allowing search to stop a rollout early and estimate the remaining future.

### 5. Search-Driven Neural-Network Training

The system randomly generates characters, support-card decks, and intermediate turns, then runs Monte Carlo search to produce policy/value supervision. The first dataset can use the hand-crafted policy for its rollouts.

After the first network is trained, later simulations use its policy output in place of the hand-crafted policy. Because the network also outputs a value, simulated runs can stop before the final turn and substitute the network's value estimate for the terminal score, greatly reducing the time cost per turn.

After being exported and installed through an external workflow, the trained network can participate in the next round of state generation and search. Repeating this process forms an approximate policy-iteration loop:

```text
hand-crafted policy → search targets → neural network → improved search targets
```

## Adapting UmaAI to a New Scenario

Adapting UmaAI to a new scenario can be divided into roughly five steps.

### 1. Analyze the Scenario and Implement Its Simulator

First, quantitatively study the mechanics of the new scenario, including but not limited to:

- formulas for stat gains from training;
- formulas for obtaining, spending, and applying scenario resources;
- probabilities of random outcomes;
- fixed-turn events and their effects;
- scenario phases, special training mechanics, and final settlement rules.

Use this research to implement a scenario simulator that reproduces the scenario quantitatively and can continue from an arbitrary intermediate state to the end of a run.

Some mechanics may be too complex or too difficult to determine precisely, such as skill score and random support-card events. These can be approximated with average gains, fixed corrections, or simplified probability models. As long as the simulator's overall deviation from the real game is modest, the resulting strategy will usually remain broadly unchanged.

### 2. Build the Game-State Data Pipeline

Implement a way to extract in-game data, convert the current turn, stats, stamina, support-character placement across training facilities, training gains, scenario resources, and other information into JSON that UmaAI can read, and reconstruct the internal `Game` state from it.

Because of compliance concerns, please determine the concrete extraction method yourself. This repository does not describe one.

### 3. Write a Hand-Crafted Policy

Start from the hand-crafted policy of an existing scenario. Keep general factors such as training gains, stamina, failure rate, and bond, then add the value of the new scenario's resources, phase goals, and special actions.

The policy does not need to be perfect, but it should not be excessively weak. As a rough target, under the same setup, its average final training score should preferably be no more than about 3,000 points below the average score of experienced players.

Run many complete simulated training runs and use the Monte Carlo average score to observe policy strength, then adjust the evaluation weights. Both the multi-run average and individual turn-level actions are useful references for tuning.

### 4. Check and Adapt the Monte Carlo Pipeline

The existing code usually provides the basic Monte Carlo framework: clone the root state, fix the first action, run the remaining rollout, calculate the terminal score, and aggregate results.

Different scenarios may expose different action sets, including multi-stage choices, resource purchases, state switches, or combined actions within one turn. Modify action encoding, legality checks, first-action execution, and candidate-result display as needed so that search covers every meaningful strategy available on the current turn.

After completing the first four steps, UmaAI is already usable. With a sufficiently accurate simulator and a competent hand-crafted policy, the combination of that policy and Monte Carlo search can usually outperform the vast majority of ordinary players, although further improvement remains possible.

### 5. Train a Neural Network (Optional)

Generate a large and comprehensive set of random intermediate states, then run Monte Carlo search on each state to obtain high-quality policy and value targets.

Each training sample mainly contains:

- **input**: a vectorized representation of the current state, including general training state and scenario-specific state;
- **policy**: the action distribution produced by the hand-crafted policy combined with Monte Carlo search;
- **value**: the mean and variation of final Monte Carlo scores, represented in the current implementation by the mean and standard deviation.

A hand-crafted policy enhanced by Monte Carlo search is usually substantially stronger than the raw hand-crafted policy. A neural network trained on these search results can therefore produce a fast standalone policy that is stronger than the original hand-crafted strategy.

After the first-generation network is trained, its policy can replace the hand-crafted policy inside Monte Carlo rollouts. The resulting “neural network + Monte Carlo” combination is usually stronger than “hand-crafted policy + Monte Carlo.” Data generated by this stronger combination can then train a second-generation network.

Considering the cost of data generation and training, along with diminishing returns from later iterations, one or two generations are usually sufficient. Additional generations tend to produce small improvements at a large computational cost.

With an accurate simulator, broad state coverage, and high-quality training, the neural-network version can generally be considered close to optimal under the current simulator and scoring objective.
